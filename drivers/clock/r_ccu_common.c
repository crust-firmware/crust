/*
 * Copyright © 2019-2022 The Crust Firmware Authors.
 * SPDX-License-Identifier: BSD-3-Clause OR GPL-2.0-only
 */

#include <counter.h>
#include <delay.h>
#include <mmio.h>
#include <stdint.h>
#include <system.h>
#include <platform/devices.h>
#include <platform/prcm.h>
#include <platform/time.h>

#include "ccu.h"

#define PLL_CTRL_REG1_MASK (PLL_CTRL_REG1_KEY_FIELD | \
	                    PLL_CTRL_REG1_CRYSTAL_EN | \
	                    PLL_CTRL_REG1_LDO_EN)

/* Persist this var as r_ccu_init() may not be called after an exception. */
static uint32_t iosc_rate = 16000000U;

DEFINE_FIXED_RATE(r_ccu_common_get_iosc_rate, iosc_rate)

/**
 * Write a value to the lockable bits in PLL_CTRL_REG1.
 */
static void
write_pll_ctrl_reg1(uint32_t new)
{
	uint32_t val = mmio_read_32(PLL_CTRL_REG1) & ~PLL_CTRL_REG1_MASK;

	/* Unlock if locked, otherwise write value. */
	mmio_write_32(PLL_CTRL_REG1, val | new | PLL_CTRL_REG1_KEY);
	/* Write value if just unlocked, otherwise write same; lock. */
	mmio_write_32(PLL_CTRL_REG1, val | new);
}

void
r_ccu_common_suspend(uint8_t depth)
{
	if (depth == SD_NONE)
		return;

	if (CONFIG(OSC24M_SRC_X24M)) {
		write_pll_ctrl_reg1(PLL_CTRL_REG1_LDO_EN);
		udelay(1);
	}
	write_pll_ctrl_reg1(0);
	if (depth == SD_OSC24M)
		return;

	mmio_set_32(VDD_SYS_PWROFF_GATING_REG, AVCC_GATING);
	if (depth == SD_AVCC)
		return;

	mmio_set_32(VDD_SYS_PWROFF_GATING_REG, VDD_CPUS_GATING);
	mmio_write_32(VDD_SYS_RESET_REG, 0);
	if (depth == SD_VDD_SYS)
		return;
}

void WEAK ATTRIBUTE(alias("r_ccu_common_suspend"))
r_ccu_suspend(uint8_t depth);

void
r_ccu_common_resume(void)
{
	/*
	 * The suspend/resume steps are incremental and idempotent. There is no
	 * need to branch based on the suspend depth; just run them all. This
	 * simplifies handling a firmware restart where the depth is unknown.
	 */
	mmio_write_32(VDD_SYS_RESET_REG, VDD_SYS_RESET);
	mmio_clr_32(VDD_SYS_PWROFF_GATING_REG, VDD_CPUS_GATING | AVCC_GATING);
	if (!mmio_get_32(PLL_CTRL_REG1, PLL_CTRL_REG1_LDO_EN)) {
		write_pll_ctrl_reg1(PLL_CTRL_REG1_LDO_EN);
		if (CONFIG(OSC24M_SRC_X24M)) {
			udelay(2000);
			write_pll_ctrl_reg1(PLL_CTRL_REG1_CRYSTAL_EN |
			                    PLL_CTRL_REG1_LDO_EN);
		}
	}
}

void WEAK ATTRIBUTE(alias("r_ccu_common_resume"))
r_ccu_resume(void);

void
r_ccu_common_init(void)
{
	uint32_t after, before, end, now;

	/* Cycle until the interval will not span a counter wraparound. */
	do {
		before = cycle_counter_read();
		barrier();
		now = system_counter_read();
		end = now + (REFCLK_HZ >> 9);
	} while (end < now);

	/* Cycle until the end of the interval. */
	do {
		after = cycle_counter_read();
		/* Ensure the counters are read in a consistent order. */
		barrier();
		now = system_counter_read();
	} while (now < end);

	/*
	 * Convert the number of IOSC cycles in 1/512 second to Hz. 512 is
	 * chosen because it is the largest power-of-two factor of 24MHz, the
	 * reference clock frequency.
	 */
	iosc_rate = (after - before) << 9;
}
