/*
 * Copyright © 2017-2018 The Crust Firmware Authors.
 * SPDX-License-Identifier: (BSD-3-Clause OR GPL-2.0)
 */

#include <compiler.h>
#include <console.h>
#include <debug.h>
#include <dm.h>
#include <scpi.h>
#include <stdbool.h>
#include <test.h>
#include <watchdog.h>
#include <work.h>
#include <platform/devices.h>
#include <platform/time.h>

#define WDOG_TIMEOUT (5 * REFCLK_HZ) /* 5 seconds */

noreturn void main(void);

noreturn void
main(void)
{
	struct device *watchdog;

	console_init(DEV_UART0);
	dm_init();
	run_tests();

	/* Enable watchdog. */
	if ((watchdog = dm_get_by_class(DM_CLASS_WATCHDOG))) {
		watchdog_enable(watchdog, WDOG_TIMEOUT);
		info("Trusted watchdog enabled");
	}

	/* Do this last, as it tells SCPI clients we are finished booting. */
	scpi_init();

	/* Process work queue. */
	while (true) {
		process_work();

		/* TODO: Enter sleep state */
	}
}
