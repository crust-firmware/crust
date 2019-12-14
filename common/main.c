/*
 * Copyright © 2017-2019 The Crust Firmware Authors.
 * SPDX-License-Identifier: BSD-3-Clause OR GPL-2.0-only
 */

#include <compiler.h>
#include <console.h>
#include <counter.h>
#include <debug.h>
#include <device.h>
#include <kconfig.h>
#include <pmic.h>
#include <scpi.h>
#include <spr.h>
#include <stdbool.h>
#include <system_power.h>
#include <watchdog.h>
#include <irq/sun4i-intc.h>
#include <msgbox/sunxi-msgbox.h>
#include <watchdog/sunxi-twd.h>
#include <platform/devices.h>
#include <platform/time.h>

#define WATCHDOG_TIMEOUT 5 /* seconds */

noreturn void main(uint32_t exception);

noreturn void
main(uint32_t exception)
{
	uint64_t next_tick = counter_read() + REFCLK_HZ;

	console_init(DEV_UART0);

	if (exception) {
		error("Unhandled exception %u at %p!",
		      exception, (void *)mfspr(SPR_SYS_EPCR_INDEX(0)));
	}

	/* Initialize and enable the watchdog first. This provides a failsafe
	 * for the possibility that something below hangs. */
	device_probe(&r_twd.dev);
	watchdog_enable(&r_twd.dev, WATCHDOG_TIMEOUT * REFCLK_HZ);
	info("Watchdog enabled");

	/* Initialize the remaining devices needed to boot. */
	device_probe(&msgbox.dev);

	/* Initialize the power management IC. */
	pmic_detect();
	device_probe(pmic);

	/* Inform SCPI clients that the firmware has finished booting. */
	scpi_init();

	while (true) {
		/* Perform every-iteration operations. */
		if (system_is_running()) {
			/* Poll communication channels. */
			msgbox.dev.drv->poll(&msgbox.dev);
		} else if (system_can_wake()) {
			/* Poll wakeup sources. */
			r_intc.dev.drv->poll(&r_intc.dev);
		}
		scpi_poll();
		system_state_machine();

		if (counter_read() > next_tick) {
			next_tick += REFCLK_HZ;

			/* Perform 1Hz operations. */
			watchdog_restart(&r_twd.dev);
		}
	}
}
