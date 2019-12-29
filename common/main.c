/*
 * Copyright © 2017-2019 The Crust Firmware Authors.
 * SPDX-License-Identifier: BSD-3-Clause OR GPL-2.0-only
 */

#include <debug.h>
#include <device.h>
#include <spr.h>
#include <system.h>
#include <watchdog.h>
#include <watchdog/sunxi-twd.h>
#include <platform/time.h>

#define WATCHDOG_TIMEOUT 5 /* seconds */

noreturn void main(uint32_t exception);

noreturn void
main(uint32_t exception)
{
	const struct device *watchdog;

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
	for (;;) {
		system_state_machine();
		if (watchdog)
			watchdog_restart(watchdog);
	}
}
