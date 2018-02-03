/*
 * Copyright © 2017-2018 The Crust Firmware Authors.
 * SPDX-License-Identifier: (BSD-3-Clause OR GPL-2.0)
 */

#include <console.h>
#include <dm.h>
#include <stdbool.h>
#include <work.h>
#include <platform/devices.h>

void main(void);

void
main(void)
{
	console_init(DEV_UART0);
	dm_init();

	while (true) {
		process_work();

		/* TODO: Enter sleep state */
	}
}
