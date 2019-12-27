/*
 * Copyright © 2019 The Crust Firmware Authors.
 * SPDX-License-Identifier: BSD-3-Clause OR GPL-2.0-only
 */

#include <counter.h>
#include <mmio.h>
#include <platform/devices.h>

#define CNT_LOW_REG  0x0
#define CNT_HIGH_REG 0x4

uint64_t
counter_read(void)
{
	uint32_t high_reg;
	uint32_t low_reg;

	/* Ensure the low word doesn't wrap around while reading it. */
	do {
		high_reg = mmio_read_32(DEV_CNT_R + CNT_HIGH_REG);
		low_reg  = mmio_read_32(DEV_CNT_R + CNT_LOW_REG);
	} while (mmio_read_32(DEV_CNT_R + CNT_HIGH_REG) != high_reg);

	return ((uint64_t)high_reg << 32) | low_reg;
}
