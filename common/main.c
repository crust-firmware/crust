/*
 * Copyright © 2017-2019 The Crust Firmware Authors.
 * SPDX-License-Identifier: BSD-3-Clause OR GPL-2.0-only
 */

#include <compiler.h>
#include <console.h>
#include <debug.h>
#include <monitoring.h>
#include <scpi.h>
#include <stdbool.h>
#include <watchdog.h>
#include <work.h>
#include <watchdog/sunxi-twd.h>
#include <platform/devices.h>
#include <platform/time.h>

#define WDOG_TIMEOUT (5 * REFCLK_HZ) /* 5 seconds */

noreturn void main(void);

noreturn void
main(void)
{
	console_init(DEV_UART0);
	dm_init();
	start_monitoring();

	/* Enable watchdog. */
	watchdog_enable(&r_twd, WDOG_TIMEOUT);
	info("Trusted watchdog enabled");

	/* Do this last, as it tells SCPI clients we are finished booting. */
	scpi_init();

	while (true) {
		/* Process work queue ad infinitum. */
		process_work();
	}
}
