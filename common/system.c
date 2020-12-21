/*
 * Copyright © 2017-2020 The Crust Firmware Authors.
 * SPDX-License-Identifier: BSD-3-Clause OR GPL-2.0-only
 */

#include <cir.h>
#include <counter.h>
#include <css.h>
#include <debug.h>
#include <delay.h>
#include <device.h>
#include <dram.h>
#include <exception.h>
#include <irq.h>
#include <pmic.h>
#include <regulator.h>
#include <regulator_list.h>
#include <scpi.h>
#include <serial.h>
#include <simple_device.h>
#include <stddef.h>
#include <system.h>
#include <watchdog.h>
#include <clock/ccu.h>
#include <gpio/sunxi-gpio.h>
#include <msgbox/sunxi-msgbox.h>
#include <watchdog/sunxi-twd.h>

#define NEXT_STATE (system_state + 2)

/**
 * The enumeration of possible system states.
 *
 * The first three states are ordered to meet two constraints:
 *   1. BOOT must be nonzero so system_state is stored in .data and
 *      reinitialized only after a SoC reset (not after a firmware restart).
 *   2. REBOOT must be AWAKE+2 for the system_wake() transition.
 *
 * The remaining values are divided into two parallel sequences. The states are
 * ordered such that adding two to the state variable advances to the next
 * step in the sequence. Paired states have similar functions and share code.
 */
enum {
	SS_AWAKE      = 0x0, /**< System is awake; rich OS is running. */
	SS_BOOT       = 0x1, /**< First firmware boot after SoC reset. */
	SS_REBOOT     = 0x2, /**< Attempting board-level (PMIC) reboot. */

	SS_SHUTDOWN   = 0x3, /**< Transition from awake to off. */
	SS_SUSPEND    = 0x4, /**< Transition from awake to asleep. */

	SS_OFF        = 0x5, /**< System is off; RAM contents are lost. */
	SS_ASLEEP     = 0x6, /**< System is asleep; RAM contents are kept. */

	SS_PRE_RESET  = 0x7, /**< Common part of reset/resume transition. */
	SS_PRE_RESUME = 0x8, /**< Common part of reset/resume transition. */

	SS_RESET      = 0x9, /**< Transition from off to boot via SoC reset. */
	SS_RESUME     = 0xa, /**< Transition from asleep to awake. */
};

/* This variable is persisted across exception restarts. */
static uint8_t system_state = SS_BOOT;

noreturn void
system_state_machine(uint32_t exception)
{
	const struct device *cir, *mailbox, *pmic, *watchdog;
	uint8_t initial_state = system_state;

	if (initial_state > SS_BOOT) {
		/*
		 * If the firmware started in any state other than BOOT or
		 * AWAKE, assume the system is off. It could be transitioning
		 * or asleep, but resetting the board after an IRQ is safer
		 * than attempting to resume in an unpredictable environment.
		 */
		system_state = SS_OFF;

		/* Clear out inactive references. */
		cir      = NULL;
		watchdog = NULL;
		mailbox  = NULL;
	} else {
		/* Otherwise, perform BOOT actions and switch to AWAKE. */
		system_state = SS_AWAKE;

		/* First, enable watchdog protection. */
		watchdog = device_get_or_null(&r_twd.dev);

		/* Perform one-time device driver initialization. */
		counter_init();
		r_ccu_init();
		ccu_init();
		css_init();
		dram_init();

		/* Acquire runtime-only devices. */
		mailbox = device_get_or_null(&msgbox.dev);
	}

	/*
	 * Initialize the serial port. Unless a preinitialized port (UART0) is
	 * selected, errors occurring before this function call will not be
	 * logged, and exceptions (such as those caused by assertion failures)
	 * could result in a silent infinite loop.
	 *
	 * Serial communication needs accurate clock frequencies. Specifically,
	 * OSC16M must be calibrated before initializing R_UART, and R_UART
	 * will only work after an exception restart while off/asleep if the
	 * calibrated OSC16M frequency is retained across the restart.
	 */
	serial_init();

	/* Log startup messages. */
	report_exception(exception);
	debug_print_sprs();

	/*
	 * If the firmware started in the initial state, assume the secure
	 * monitor is waiting for an SCP_READY message.  Otherwise, assume
	 * nothing is listening, and skip sending the SCP_READY message to
	 * avoid filling up the mailbox.
	 */
	if (mailbox && initial_state == SS_BOOT) {
		scpi_create_message(mailbox, SCPI_CLIENT_EL3,
		                    SCPI_CMD_SCP_READY);
	}

	for (;;) {
		switch (system_state) {
		case SS_AWAKE:
			/* Poll runtime devices. */
			if (watchdog)
				watchdog_restart(watchdog);

			/* Poll runtime services. */
			if (mailbox)
				scpi_poll(mailbox);

			break;
		case SS_SHUTDOWN:
		case SS_SUSPEND:
			debug("Suspending...");

			/* Release runtime-only devices. */
			device_put(mailbox), mailbox = NULL;

			/* Synchronize device state with Linux. */
			simple_device_sync(&pio);
			simple_device_sync(&r_pio);

			/* Acquire wakeup sources. */
			cir = cir_get();

			/* Configure the SoC for minimal power consumption. */
			dram_suspend();
			ccu_suspend();

			/*
			 * Disable watchdog protection. Once devices outside
			 * the SoC (oscillators and regulators) are disabled,
			 * the watchdog cannot successfully reset the SoC.
			 */
			device_put(watchdog), watchdog = NULL;

			/* Gate the rest of the SoC before removing power. */
			r_ccu_suspend();

			/* Perform PMIC-specific actions. */
			if ((pmic = pmic_get())) {
				if (system_state == SS_SHUTDOWN &&
				    CONFIG(PMIC_SHUTDOWN))
					pmic_shutdown(pmic);
				else
					pmic_suspend(pmic);
			}

			/* Turn off all unnecessary power domains. */
			regulator_disable(&cpu_supply);

			/*
			 * The regulator provider is often part of the same
			 * device as the PMIC. Reduce churn by doing both PMIC
			 * and regulator actions before releasing the PMIC.
			 */
			device_put(pmic);

			debug("Suspend complete!");

			/* The system is now off or asleep. */
			system_state = NEXT_STATE;
			break;
		case SS_OFF:
		case SS_ASLEEP:
			debug_monitor();
			debug_print_battery();

			/* Poll wakeup sources. Reset or resume on wakeup. */
			if (cir_poll(cir) || irq_poll())
				system_state = NEXT_STATE;

			/* This must run last so the state change is seen. */
			debug_print_latency(system_state);
			break;
		case SS_PRE_RESET:
		case SS_PRE_RESUME:
			/*
			 * Perform PMIC-specific resume actions.
			 * The PMIC is expected to restore regulator state.
			 * If it fails, manually turn the regulators back on.
			 */
			if (!(pmic = pmic_get()) || pmic_resume(pmic))
				regulator_enable(&cpu_supply);
			device_put(pmic);

			/* Give regulator outputs time to rise. */
			udelay(5000);

			/* Restore SoC-internal power domains. */
			r_ccu_resume();

			/* Enable watchdog protection. */
			watchdog = device_get_or_null(&r_twd.dev);

			/* The system is now ready to reset or resume. */
			system_state = NEXT_STATE;
			break;
		case SS_RESUME:
			debug("Resuming...");

			/* Configure the SoC for full functionality. */
			ccu_resume();
			dram_resume();

			/* Release wakeup sources. */
			device_put(cir), cir = NULL;

			/* Acquire runtime-only devices. */
			mailbox = device_get_or_null(&msgbox.dev);

			/* Resume execution on the first CPU in the CSS. */
			css_set_power_state(0, 0, SCPI_CSS_ON,
			                    SCPI_CSS_ON, SCPI_CSS_ON);

			debug("Resume complete!");

			/* The system is now awake. */
			system_state = SS_AWAKE;
			break;
		case SS_REBOOT:
			/* Attempt to reset the board using the PMIC. */
			if ((pmic = pmic_get())) {
				pmic_reset(pmic);
				device_put(pmic);
			}

			/* Continue through to resetting the SoC. */
			fallthrough;
		case SS_RESET:
			/* Attempt to reset the SoC using the watchdog. */
			if (watchdog)
				watchdog_set_timeout(watchdog, 1);

			/* Continue making reboot/reset attempts. */
			break;
		default:
			unreachable();
		}
	}
}

void
system_reboot(void)
{
	/* This transition skips PRE_RESET, so the system must be awake. */
	assert(system_state == SS_AWAKE);

	system_state = SS_REBOOT;
}

void
system_reset(void)
{
	/* This transition skips PRE_RESET, so the system must be awake. */
	assert(system_state == SS_AWAKE);

	system_state = SS_RESET;
}

void
system_shutdown(void)
{
	/* This transition only makes sense when the system is awake. */
	assert(system_state == SS_AWAKE);

	system_state = SS_SHUTDOWN;
}

void
system_suspend(void)
{
	/* This transition only makes sense when the system is awake. */
	assert(system_state == SS_AWAKE);

	system_state = SS_SUSPEND;
}

void
system_wake(void)
{
	/*
	 * Initiate a state transition sequence, starting from
	 * any of the three stable states:
	 *   1. AWAKE  => REBOOT    [=> RESET] => BOOT => AWAKE
	 *   2. OFF    => PRE_RESET  => RESET  => BOOT => AWAKE
	 *   3. ASLEEP => PRE_RESUME => RESUME         => AWAKE
	 */
	system_state = NEXT_STATE;
}
