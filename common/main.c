/*
 * Copyright © 2017 Samuel Holland <samuel@sholland.org>
 * SPDX-License-Identifier: (BSD-3-Clause OR GPL-2.0)
 */

#include <console.h>
#include <platform/devices.h>

void main(void);

void
main(void)
{
	console_init(DEV_UART0);
}
