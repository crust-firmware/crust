/*
 * Copyright © 2017-2018 The Crust Firmware Authors.
 * SPDX-License-Identifier: (BSD-3-Clause OR GPL-2.0)
 */

#include <compiler.h>
#include <css.h>
#include <debug.h>
#include <delay.h>
#include <error.h>
#include <mmio.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <util.h>
#include <platform/devices.h>

#define CLUSTER_MAX                 1
#define CORE_MAX                    4

/* c = cluster, n = core. */
#define CPUCFG_CLUSTER_CTRL0_REG(c) (0x0000 + 0x10 * (c))
#define CPUCFG_CLUSTER_CTRL1_REG(c) (0x0004 + 0x10 * (c))
#define CPUCFG_CACHE_CFG0_REG       0x0008
#define CPUCFG_CACHE_CFG1_REG       0x000c
#define CPUCFG_DEBUG_REG            0x0020
#define CPUCFG_GENERAL_CTRL_REG     0x0028
#define CPUCFG_CPU_STATUS_REG(c)    (0x0030 + 0x04 * (c))
#define CPUCFG_L2_STATUS_REG        0x003c
#define CPUCFG_RESET_CTRL_REG(c)    (0x0080 + 0x04 * (c))
#define CPUCFG_RVBAR_LO_REG(c, n)   (0x00a0 + 0x20 * (c) + 0x08 * (n))
#define CPUCFG_RVBAR_HI_REG(c, n)   (0x00a4 + 0x20 * (c) + 0x08 * (n))

#define R_CPUCFG_ARISC_RESET_REG    0x0000
#define R_CPUCFG_PWRON_RESET_REG(c) (0x0030 + 0x04 * (c))
#define R_CPUCFG_SYS_RESET_REG      0x0140
#define R_CPUCFG_SS_FLAG_REG        0x01a0
#define R_CPUCFG_CPU_ENTRY_REG      0x01a4
#define R_CPUCFG_SS_ENTRY_REG       0x01a8
#define R_CPUCFG_HP_FLAG_REG        0x01ac

#define R_PRCM_PWROFF_GATING_REG(c) (0x0100 + 0x04 * (c))
#define R_PRCM_PWR_CLAMP_REG(c, n)  (0x0140 + 0x10 * (c) + 0x04 * (n))

static const uint8_t power_switch_off_sequence[] = {
	0xff,
};

static const uint8_t power_switch_on_sequence[] = {
	0xfe, 0xf8, 0xe0, 0x80, 0x00,
};

/**
 * Spin until the bits in a register match an expected mask.
 *
 * @param address The address of the MMIO register.
 * @param mask    The bits that must all be set in the register.
 */
static void
poll_bits(uintptr_t address, uint32_t mask)
{
	while ((mmio_read32(address) & mask) != mask) {
		/* Wait for the bits to go high. */
	}
}

/**
 * Enable or disable power to a core or cluster power domain. The power switch
 * for core 0 controls power to the entire cluster. Other core switches control
 * only that core.
 *
 * @param cluster The cluster to control power for.
 * @param core    The core to control power for.
 * @param enable  Whether to enable or disable the power switch.
 */
static void
set_power_switch(uint8_t cluster, uint8_t core, bool enable)
{
	const uint8_t *values;
	uint32_t length, reg;

	if (enable) {
		values = power_switch_on_sequence;
		length = ARRAY_SIZE(power_switch_on_sequence);
	} else {
		values = power_switch_off_sequence;
		length = ARRAY_SIZE(power_switch_off_sequence);
	}

	/* Avoid killing the power if the switch is already enabled. */
	reg = mmio_read32(DEV_R_PRCM + R_PRCM_PWR_CLAMP_REG(cluster, core));
	if (reg == values[length - 1])
		return;

	for (size_t i = 0; i < length; ++i) {
		mmio_write32(DEV_R_PRCM + R_PRCM_PWR_CLAMP_REG(cluster, core),
		             values[i]);
		/* Allwinner's blob uses 10, 20, and 30μs delays, depending on
		 * the iteration. However, the same code works fine in ATF with
		 * no delays. The 10μs delay is here just to be extra safe. */
		udelay(10);
	}
}

uint8_t
css_get_cluster_count(void)
{
	return CLUSTER_MAX;
}

uint8_t
css_get_core_count(uint8_t cluster __unused)
{
	return CORE_MAX;
}

uint8_t
css_get_core_state(uint8_t cluster, uint8_t core)
{
	uint32_t reg;

	assert(cluster < CLUSTER_MAX);
	assert(core < CORE_MAX);

	/* Is the core in power-on reset? */
	reg = mmio_read32(DEV_R_CPUCFG + R_CPUCFG_PWRON_RESET_REG(cluster));
	if (!(reg & BIT(core)))
		return POWER_STATE_OFF;
	/* Is the core in core reset? */
	reg = mmio_read32(DEV_CPUCFG + CPUCFG_RESET_CTRL_REG(cluster));
	if (!(reg & BIT(core)))
		return POWER_STATE_OFF;

	return POWER_STATE_ON;
}

int
css_set_core_state(uint8_t cluster, uint8_t core, uint8_t state)
{
	uint8_t current_state = css_get_core_state(cluster, core);

	assert(cluster < CLUSTER_MAX);
	assert(core < CORE_MAX);

	if (state == current_state)
		return SUCCESS;

	if (state == POWER_STATE_ON) {
		/* Deassert DBGPWRDUP (prevent debug access to the core). */
		mmio_clearbits32(DEV_CPUCFG + CPUCFG_DEBUG_REG, BIT(core));
		/* Assert core reset (active-low). */
		mmio_clearbits32(DEV_CPUCFG +
		                 CPUCFG_RESET_CTRL_REG(cluster), BIT(core));
		/* Assert core power-on reset (active-low). */
		mmio_clearbits32(DEV_R_CPUCFG +
		                 R_CPUCFG_PWRON_RESET_REG(cluster), BIT(core));
		/* Program the core to start in AArch64 mode. */
		mmio_setbits32(DEV_CPUCFG + CPUCFG_CLUSTER_CTRL0_REG(cluster),
		               BIT(24 + core));
		/* Core 0 does not have a separate power domain. */
		if (core > 0) {
			/* Turn on power to the core power domain. */
			set_power_switch(cluster, core, true);
			/* Release the core output clamps. */
			mmio_clearbits32(DEV_R_PRCM +
			                 R_PRCM_PWROFF_GATING_REG(cluster),
			                 BIT(core));
		}
		/* Deassert core power-on reset (active-low). */
		mmio_setbits32(DEV_R_CPUCFG +
		               R_CPUCFG_PWRON_RESET_REG(cluster), BIT(core));
		/* Deassert core reset (active-low). */
		mmio_setbits32(DEV_CPUCFG +
		               CPUCFG_RESET_CTRL_REG(cluster), BIT(core));
		/* Assert DBGPWRDUP (allow debug access to the core). */
		mmio_setbits32(DEV_CPUCFG + CPUCFG_DEBUG_REG, BIT(core));
	} else if (state == POWER_STATE_OFF) {
		/* Wait for the core to be in WFI and ready to shut down. */
		poll_bits(DEV_CPUCFG +
		          CPUCFG_CPU_STATUS_REG(cluster), BIT(16 + core));
		/* Deassert DBGPWRDUP (prevent debug access to the core). */
		mmio_clearbits32(DEV_CPUCFG + CPUCFG_DEBUG_REG, BIT(core));
		/* Core 0 does not have a separate power domain. */
		if (core > 0) {
			/* Activate the core output clamps. */
			mmio_setbits32(DEV_R_PRCM +
			               R_PRCM_PWROFF_GATING_REG(cluster),
			               BIT(core));
		}
		/* Assert core reset (active-low). */
		mmio_clearbits32(DEV_CPUCFG +
		                 CPUCFG_RESET_CTRL_REG(cluster), BIT(core));
		/* Assert core power-on reset (active-low). */
		mmio_clearbits32(DEV_R_CPUCFG +
		                 R_CPUCFG_PWRON_RESET_REG(cluster), BIT(core));
		/* Core 0 does not have a separate power domain. */
		if (core > 0) {
			/* Remove power from the core power domain. */
			set_power_switch(cluster, core, false);
		}
	} else {
		/* Unknown power state requested. */
		return EINVAL;
	}

	return SUCCESS;
}
