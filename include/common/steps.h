/*
 * Copyright © 2021 The Crust Firmware Authors.
 * SPDX-License-Identifier: BSD-3-Clause OR GPL-2.0-only
 */

#ifndef COMMON_STEPS_H
#define COMMON_STEPS_H

#include <stdint.h>
#include <version.h>

#define STEP_KIND(n) (VERSION_MAJOR << 12 | VERSION_MINOR << 8 | (n) << 4)

enum {
	STEP_NONE,
	STEP_SUSPEND = STEP_KIND(0),
	STEP_SUSPEND_CORE,
	STEP_SUSPEND_CLUSTER,
	STEP_SUSPEND_CSS,
	STEP_SUSPEND_DEVICES,
	STEP_SUSPEND_DRAM,
	STEP_SUSPEND_CCU,
	STEP_SUSPEND_PRCM,
	STEP_SUSPEND_PMIC,
	STEP_SUSPEND_REGULATORS,
	STEP_SUSPEND_COMPLETE,
	STEP_RESUME = STEP_KIND(1),
	STEP_RESUME_PMIC,
	STEP_RESUME_REGULATORS,
	STEP_RESUME_PRCM,
	STEP_RESUME_CCU,
	STEP_RESUME_DRAM,
	STEP_RESUME_DRAM_CHECKSUM,
	STEP_RESUME_DEVICES,
	STEP_RESUME_CSS,
	STEP_RESUME_COMPLETE,
};

#if CONFIG(DEBUG_RECORD_STEPS)

void record_exception(uint32_t exception, uint32_t pc);
void record_step(uint32_t step);
void report_last_step(void);

#else

static inline void
record_exception(uint32_t exception UNUSED, uint32_t pc UNUSED)
{
}

static inline void
record_step(uint32_t step UNUSED)
{
}

static inline void
report_last_step(void)
{
}

#endif

#endif /* COMMON_STEPS_H */
