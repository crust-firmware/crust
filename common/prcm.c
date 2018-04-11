/*
 * Copyright © 2017-2018 The Crust Firmware Authors.
 * SPDX-License-Identifier: (BSD-3-Clause OR GPL-2.0)
 */

#include <compiler.h>
#include <error.h>
#include <prcm.h>
#include <stdint.h>

int __weak
prcm_shutdown(void)
{
	/* Generic, non-platform specific return value. */
	return ENOTSUP;
}
