/*
 * Copyright © 2021 The Crust Firmware Authors.
 * SPDX-License-Identifier: BSD-3-Clause OR GPL-2.0-only
 */

#include <debug.h>
#include <mmio.h>
#include <steps.h>
#include <platform/devices.h>

#define RTC_GP_DATA_REG(n) (DEV_RTC + 0x0100 + 0x4 * (n))
#define LAST_EXCEPTION_REG RTC_GP_DATA_REG(2)
#define LAST_STEP_REG      RTC_GP_DATA_REG(3)

void
record_exception(uint32_t exception, uint32_t pc)
{
	mmio_write_32(LAST_EXCEPTION_REG, exception << 24 | pc);
}

void
record_step(uint32_t step)
{
	mmio_write_32(LAST_STEP_REG, step);
}

void
report_last_step(void)
{
	uint32_t step = mmio_read_32(LAST_STEP_REG);

	if (step != STEP_NONE)
		error("Step %04x failed!", step);
}
