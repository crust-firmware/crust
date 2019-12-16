/*
 * Copyright © 2017-2019 The Crust Firmware Authors.
 * SPDX-License-Identifier: BSD-3-Clause OR GPL-2.0-only
 */

#include <counter.h>
#include <debug.h>
#include <device.h>
#include <scpi.h>
#include <spr.h>
#include <system_power.h>
#include <watchdog.h>
#include <watchdog/sunxi-twd.h>
#include <platform/time.h>

#define WATCHDOG_TIMEOUT 5 /* seconds */

noreturn void main(uint32_t exception);

noreturn void
main(uint32_t exception)
{
	const struct device *watchdog;
	uint64_t next_tick;

	if (exception) {
		error("Unhandled exception %u at %p!",
		      exception, (void *)mfspr(SPR_SYS_EPCR_INDEX(0)));
	}

	/* Initialize and enable the watchdog first. This provides a failsafe
	 * for the possibility that something below hangs. */
	if ((watchdog = device_get(&r_twd.dev))) {
		watchdog_enable(watchdog, WATCHDOG_TIMEOUT * REFCLK_HZ);
		info("Watchdog enabled");
	}

	/* Initialize the power management state machine. */
	system_state_init();

	/* Inform SCPI clients that the firmware has finished booting. */
	scpi_init();

	next_tick = counter_read() + REFCLK_HZ;
	for (;;) {
		/* Perform every-iteration operations. */
		scpi_poll();
		system_state_machine();

		if (counter_read() > next_tick) {
			next_tick += REFCLK_HZ;

			/* Perform 1Hz operations. */
			if (watchdog)
				watchdog_restart(watchdog);
		}
	}
}
