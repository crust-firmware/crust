/*
 * Copyright © 2017-2019 The Crust Firmware Authors.
 * SPDX-License-Identifier: BSD-3-Clause OR GPL-2.0-only
 */

#include <compiler.h>
#include <console.h>
#include <debug.h>
#include <dm.h>
#include <kconfig.h>
#include <pmic.h>
#include <scpi.h>
#include <stdbool.h>
#include <wallclock.h>
#include <watchdog.h>
#include <irq/sun4i-intc.h>
#include <misc/gpio-button.h>
#include <msgbox/sunxi-msgbox.h>
#include <watchdog/sunxi-twd.h>
#include <platform/devices.h>
#include <platform/time.h>

#define WATCHDOG_TIMEOUT 5 /* seconds */

noreturn void main(void);

noreturn void
main(void)
{
	uint64_t next_tick = wallclock_read() + REFCLK_HZ;

	console_init(DEV_UART0);

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

	/* Initialize wakeup sources. */
	if (IS_ENABLED(CONFIG_GPIO_BUTTON))
		device_probe(&power_button.dev);

	/* Inform SCPI clients that the firmware has finished booting. */
	scpi_init();

	while (true) {
		/* Perform every-iteration operations. */
		msgbox.dev.drv->poll(&msgbox.dev);
		r_intc.dev.drv->poll(&r_intc.dev);
		scpi_poll();

		if (wallclock_read() > next_tick) {
			next_tick += REFCLK_HZ;

			/* Perform 1Hz operations. */
			watchdog_restart(&r_twd.dev);
		}
	}
}
