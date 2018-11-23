/*
 * Copyright © 2017-2018 The Crust Firmware Authors.
 * SPDX-License-Identifier: BSD-3-Clause OR GPL-2.0-only
 */

#include <compiler.h>
#include <mmio.h>
#include <util.h>
#include <wallclock.h>
#include <platform/devices.h>

#define CNT64_CTRL_REG 0x280
#define CNT64_LOW_REG  0x284
#define CNT64_HIGH_REG 0x288

uint64_t __weak
wallclock_read(void)
{
	uint32_t high_reg;
	uint32_t low_reg;

	mmio_setbits32(DEV_R_CPUCFG + CNT64_CTRL_REG, BIT(1));
	while (mmio_read32(DEV_R_CPUCFG + CNT64_CTRL_REG) & BIT(1)) {
		/* Wait for the counter to latch. */
	}
	high_reg = mmio_read32(DEV_R_CPUCFG + CNT64_HIGH_REG);
	low_reg  = mmio_read32(DEV_R_CPUCFG + CNT64_LOW_REG);

	return ((uint64_t)high_reg << 32) | low_reg;
}
