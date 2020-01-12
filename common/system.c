/*
 * Copyright © 2017-2019 The Crust Firmware Authors.
 * SPDX-License-Identifier: BSD-3-Clause OR GPL-2.0-only
 */

#include <css.h>
#include <debug.h>
#include <device.h>
#include <irq.h>
#include <pmic.h>
#include <scpi.h>
#include <stdbool.h>
#include <system.h>
#include <util.h>
#include <watchdog.h>
#include <watchdog/sunxi-twd.h>
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
	const struct device *pmic, *watchdog;
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
	} else {
		system_state = SYSTEM_ACTIVE;

		/* Initialize runtime devices. */
		if ((watchdog = device_get(&r_twd.dev)))
			watchdog_enable(watchdog, WATCHDOG_TIMEOUT);

		/* Initialize runtime services. */
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

			/* Turn off all unnecessary clocks. */
			if (watchdog) {
				device_put(watchdog);
				watchdog = NULL;
			}

			debug("Suspend complete!");

			/* The system is now inactive. */
			system_state = SYSTEM_INACTIVE;
			break;
		case SYSTEM_INACTIVE:
			/* Poll wakeup sources. Resume on wakeup. */
			if (irq_poll())
				system_state = SYSTEM_RESUME;
			break;
		case SYSTEM_RESUME:
			debug("Resuming...");

			/* Turn on previously-disabled clocks. */
			if ((watchdog = device_get(&r_twd.dev)))
				watchdog_enable(watchdog, WATCHDOG_TIMEOUT);

			/* Turn on previously-disabled power domains. */

			/* Perform PMIC-specific resume actions. */
			if ((pmic = pmic_get())) {
				pmic_resume(pmic);
				device_put(pmic);
			}

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

			/* Turn off all possible clocks. */
			if (watchdog) {
				device_put(watchdog);
				watchdog = NULL;
			}

			/* The system is now off. */
			system_state = SYSTEM_OFF;
			break;
		case SYSTEM_OFF:
			/* Poll wakeup sources. Reset on wakeup. */
			if (!irq_poll())
				system_state = SYSTEM_RESET;
			break;
		default: /* SYSTEM_RESET and all others */
			/* Attempt to reset the SoC using the PMIC. */
			if ((pmic = pmic_get())) {
				pmic_reset(pmic);
				device_put(pmic);
			}

			/* Attempt to reset the SoC using the watchdog. */
			if (watchdog || (watchdog = device_get(&r_twd.dev)))
				watchdog_enable(watchdog, 1);

			/* Leave the system state as is; continue making reset
			 * attempts each time this function is called. */
			break;
		}

		debug_print_latency();
	}
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
