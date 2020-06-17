/*
 * Copyright © 2017-2020 The Crust Firmware Authors.
 * SPDX-License-Identifier: BSD-3-Clause OR GPL-2.0-only
 */

#include <css.h>
#include <debug.h>
#include <delay.h>
#include <device.h>
#include <error.h>
#include <irq.h>
#include <pmic.h>
#include <regulator.h>
#include <regulator_list.h>
#include <scpi.h>
#include <stdbool.h>
#include <stddef.h>
#include <system.h>
#include <util.h>
#include <watchdog.h>
#include <gpio/sunxi-gpio.h>
#include <watchdog/sunxi-twd.h>
#include <platform/irq.h>
#include <platform/time.h>

#define WATCHDOG_TIMEOUT (5 * REFCLK_HZ) /* 5 seconds */

static uint8_t system_state;

uint8_t
get_system_state(void)
{
	return system_state;
}

bool
system_can_wake(void)
{
	return system_state == SYSTEM_INACTIVE ||
	       system_state == SYSTEM_OFF;
}

bool
system_is_running(void)
{
	return system_state == SYSTEM_ACTIVE;
}

noreturn void
system_state_machine(void)
{
	const struct device *gpio, *pmic, *watchdog;
	uint8_t cpus;

	/*
	 * If no CPU is online, assume the system is off. It could be
	 * suspended, but resetting the board is safer than attempting to
	 * resume in an unpredictable environment. Otherwise, prepare the
	 * SYSTEM_ACTIVE state.
	 */
	cpus = css_get_online_cores(0);
	if (!cpus) {
		system_state = SYSTEM_OFF;

		/* Clear out inactive references. */
		watchdog = NULL;
		gpio     = NULL;
	} else {
		system_state = SYSTEM_ACTIVE;

		/* Initialize runtime devices. */
		if ((watchdog = device_get_or_null(&r_twd.dev)))
			watchdog_enable(watchdog, WATCHDOG_TIMEOUT);
		gpio = device_get_or_null(&r_pio.dev);

		/* Initialize runtime services. */
		css_init();
		scpi_init();

		/*
		 * If only CPU0 is online, assume Linux has not booted yet, and
		 * Trusted Firmware is waiting for an SCP_READY message. Skip
		 * sending SCP_READY otherwise, to avoid filling up the mailbox
		 * when nothing is listening.
		 */
		if (cpus == BIT(0)) {
			scpi_create_message(SCPI_CLIENT_EL3,
			                    SCPI_CMD_SCP_READY);
		}
	}

	for (;;) {
		switch (system_state) {
		case SYSTEM_ACTIVE:
			/* Poll runtime devices. */
			if (watchdog)
				watchdog_restart(watchdog);

			/* Poll runtime services. */
			scpi_poll();

			break;
		case SYSTEM_SUSPEND:
			debug("Suspending...");

			/* Disable runtime services. */
			scpi_exit();

			/* Enable wakeup sources. */

			/* Perform PMIC-specific suspend actions. */
			if ((pmic = pmic_get())) {
				pmic_suspend(pmic);
				device_put(pmic);
			}

			/* Turn off all unnecessary power domains. */
			regulator_bulk_disable(&inactive_list);

			/* Turn off all unnecessary clocks. */
			if (!irq_is_enabled(IRQ_R_PIO_PL)) {
				device_put(gpio);
				gpio = NULL;
			}
			device_put(watchdog);
			watchdog = NULL;

			debug("Suspend complete!");

			/* The system is now inactive. */
			system_state = SYSTEM_INACTIVE;
			break;
		case SYSTEM_INACTIVE:
			/* Poll wakeup sources. Resume on wakeup. */
			if (!irq_poll())
				break;
			fallthrough;
		case SYSTEM_RESUME:
			debug("Resuming...");

			/* Turn on previously-disabled clocks. */
			if ((watchdog = device_get_or_null(&r_twd.dev)))
				watchdog_enable(watchdog, WATCHDOG_TIMEOUT);
			if (!gpio)
				gpio = device_get_or_null(&r_pio.dev);

			/* Turn on previously-disabled power domains. */

			/* Perform PMIC-specific resume actions.
			 * The PMIC is expected to restore regulator state. */
			bool restored = false;
			if ((pmic = pmic_get())) {
				restored = pmic_resume(pmic) == SUCCESS;
				device_put(pmic);
			}
			if (!restored)
				regulator_bulk_enable(&inactive_list);

			/* Give regulator outputs time to rise. */
			udelay(5000);

			/* Disable wakeup sources. */

			/* Enable runtime services. */
			scpi_init();

			/* Resume execution on the first CPU in the CSS. */
			css_set_css_state(SCPI_CSS_ON);
			css_set_cluster_state(0, SCPI_CSS_ON);
			css_set_core_state(0, 0, SCPI_CSS_ON);

			debug("Resume complete!");

			/* The system is now active. */
			system_state = SYSTEM_ACTIVE;
			break;
		case SYSTEM_SHUTDOWN:
			/* Disable runtime services. */
			scpi_exit();

			/* Enable a subset of wakeup sources. */

			/* Perform PMIC-specific shutdown actions. */
			if ((pmic = pmic_get())) {
				pmic_shutdown(pmic);
				device_put(pmic);
			}

			/* Turn off all possible power domains. */
			regulator_bulk_disable(&off_list);

			/* Turn off all possible clocks. */
			if (!irq_is_enabled(IRQ_R_PIO_PL)) {
				device_put(gpio);
				gpio = NULL;
			}
			device_put(watchdog);
			watchdog = NULL;

			/* The system is now off. */
			system_state = SYSTEM_OFF;
			break;
		case SYSTEM_OFF:
			/* Poll wakeup sources. Reboot on wakeup. */
			if (!irq_poll())
				break;
			fallthrough;
		case SYSTEM_REBOOT:
			/* Attempt to reset the board using the PMIC. */
			if ((pmic = pmic_get())) {
				pmic_reset(pmic);
				device_put(pmic);
			}

			/* Continue through to resetting the SoC. */
			fallthrough;
		case SYSTEM_RESET:
		default:
			/* Turn on regulators required to boot. */
			regulator_bulk_enable(&off_list);

			/* Give regulator outputs time to rise. */
			udelay(5000);

			/* Attempt to reset the SoC using the watchdog. */
			if (!watchdog)
				watchdog = device_get_or_null(&r_twd.dev);
			if (watchdog)
				watchdog_enable(watchdog, 1);

			/* Continue making reset attempts each iteration. */
			system_state = SYSTEM_RESET;
			break;
		}

		debug_monitor();
		debug_print_battery();
		debug_print_latency();
	}
}

void
system_reboot(void)
{
	system_state = SYSTEM_REBOOT;
}

void
system_reset(void)
{
	system_state = SYSTEM_RESET;
}

void
system_shutdown(void)
{
	if (system_state == SYSTEM_ACTIVE)
		system_state = SYSTEM_SHUTDOWN;
}

void
system_suspend(void)
{
	if (system_state == SYSTEM_ACTIVE)
		system_state = SYSTEM_SUSPEND;
}

void
system_wakeup(void)
{
	if (system_state == SYSTEM_INACTIVE)
		system_state = SYSTEM_RESUME;
	if (system_state == SYSTEM_OFF)
		system_state = SYSTEM_RESET;
}
