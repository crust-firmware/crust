/*
 * Copyright © 2022 The Crust Firmware Authors.
 * SPDX-License-Identifier: BSD-3-Clause OR GPL-2.0-only
 */

#include <counter.h>
#include <mmio.h>
#include <platform/devices.h>

#define CNT_LO_REG (DEV_TIMESTAMP_STATUS + 0x0)
#define CNT_HI_REG (DEV_TIMESTAMP_STATUS + 0x4)

uint32_t
system_counter_read(void)
{
	return mmio_read_32(CNT_LO_REG);
}
