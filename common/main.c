/*
 * Copyright © 2017-2018 The Crust Firmware Authors.
 * SPDX-License-Identifier: BSD-3-Clause OR GPL-2.0-only
 */

#include <compiler.h>
#include <console.h>
#include <debug.h>
#include <dm.h>
#include <monitoring.h>
#include <scpi.h>
#include <stdbool.h>
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
	start_monitoring();

	/* Enable watchdog. */
	if ((watchdog = dm_first_dev_by_class(DM_CLASS_WATCHDOG))) {
		watchdog_enable(watchdog, WDOG_TIMEOUT);
		info("Trusted watchdog enabled");
	}

	/* Do this last, as it tells SCPI clients we are finished booting. */
	scpi_init();

	while (true) {
		/* Process work queue ad infinitum. */
		process_work();
	}
}
