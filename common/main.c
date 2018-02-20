/*
 * Copyright © 2017-2018 The Crust Firmware Authors.
 * SPDX-License-Identifier: (BSD-3-Clause OR GPL-2.0)
 */

#include <compiler.h>
#include <console.h>
#include <debug.h>
#include <dm.h>
#include <stdbool.h>
#include <work.h>
#include <drivers/watchdog.h>
#include <platform/devices.h>

#define WDOG_TIMEOUT (5 * 1000 * 1000 * 24) /* 5 seconds */

noreturn void main(void);

noreturn void
main(void)
{
	struct device *watchdog;

	console_init(DEV_UART0);
	dm_init();

	/* Enable watchdog. */
	if ((watchdog = dm_get_by_class(DM_CLASS_WATCHDOG))) {
		watchdog_enable(watchdog, WDOG_TIMEOUT);
		info("Trusted watchdog enabled");
	}

	/* Process work queue. */
	while (true) {
		process_work();

		/* TODO: Enter sleep state */
	}
}
