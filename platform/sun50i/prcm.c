/*
 * Copyright © 2017-2018 The Crust Firmware Authors.
 * SPDX-License-Identifier: (BSD-3-Clause OR GPL-2.0)
 */

#include <dm.h>
#include <error.h>
#include <mmio.h>
#include <prcm.h>
#include <platform/devices.h>

#define VDD_SYS_PWROFF_GATING (DEV_R_PRCM + 0x0110)

int
prcm_shutdown(void)
{
	/* Activate power gating for shutdown. */
	mmio_setbits32(VDD_SYS_PWROFF_GATING, BITMASK(0, 2));
	mmio_setbits32(VDD_SYS_PWROFF_GATING, BITMASK(3, 2));
	mmio_setbits32(VDD_SYS_PWROFF_GATING, BIT(8));
	mmio_setbits32(VDD_SYS_PWROFF_GATING, BIT(12));

	return SUCCESS;
}
