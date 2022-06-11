/*
 * Copyright © 2022 The Crust Firmware Authors.
 * SPDX-License-Identifier: BSD-3-Clause OR GPL-2.0-only
 */

#include <counter.h>
#include <mmio.h>
#include <platform/cpucfg.h>

uint32_t
system_counter_read(void)
{
	mmio_write_32(CNT64_CTRL_REG, CNT64_RL_EN);
	mmio_pollz_32(CNT64_CTRL_REG, CNT64_RL_EN);

	return mmio_read_32(CNT64_LO_REG);
}
