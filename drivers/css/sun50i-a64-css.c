/*
 * Copyright © 2017-2020 The Crust Firmware Authors.
 * SPDX-License-Identifier: BSD-3-Clause OR GPL-2.0-only
 */

#include <css.h>
#include <delay.h>
#include <mmio.h>
#include <scpi_protocol.h>
#include <stdint.h>
#include <util.h>
#include <platform/devices.h>
#include <platform/prcm.h>

#include "css.h"

#define CLUSTER_CTRL_REG0         (DEV_CPUCFG + 0x0000)
#define CLUSTER_CTRL_REG1         (DEV_CPUCFG + 0x0004)
#define CACHE_CFG_REG0            (DEV_CPUCFG + 0x0008)
#define CACHE_CFG_REG1            (DEV_CPUCFG + 0x000c)
#define DEBUG_REG0                (DEV_CPUCFG + 0x0020)
#define GENERAL_CTRL_REG0         (DEV_CPUCFG + 0x0028)
#define CPU_STATUS_REG            (DEV_CPUCFG + 0x0030)
#define L2_STATUS_REG             (DEV_CPUCFG + 0x003c)
#define CLUSTER_RESET_CTRL_REG    (DEV_CPUCFG + 0x0080)
#define RVBA_LO_REG(n)            (DEV_CPUCFG + 0x00a0 + 0x08 * (n))
#define RVBA_HI_REG(n)            (DEV_CPUCFG + 0x00a4 + 0x08 * (n))

#define CLUSTER_PWRON_RESET_REG   (DEV_R_CPUCFG + 0x0030)
#define CPU_SYS_RESET_REG         (DEV_R_CPUCFG + 0x0140)

#define CLUSTER_PWROFF_GATING_REG (DEV_R_PRCM + 0x0100)
#define CPU_PWR_CLAMP_REG(n)      (DEV_R_PRCM + 0x0140 + 0x04 * (n))

/* Reset Vector Base Address. */
static uint32_t rvba;

int
css_set_css_state(uint32_t state UNUSED)
{
	/* Nothing to do. */
	return SCPI_OK;
}

int
css_set_cluster_state(uint32_t cluster UNUSED, uint32_t state)
{
	if (state == SCPI_CSS_ON) {
		/* Apply power to the cluster power domain. */
		css_set_power_switch(CPU_PWR_CLAMP_REG(0), true);
		/* Release the cluster output clamps. */
		mmio_clr_32(CLUSTER_PWROFF_GATING_REG, BIT(0));
		udelay(1);
		/* Deassert an undocumented reset bit (active-low). */
		mmio_set_32(CPU_SYS_RESET_REG, BIT(0));
		udelay(1);
		/* Assert all cluster resets (active-low). */
		mmio_write_32(CLUSTER_RESET_CTRL_REG, 0);
		/* Enable hardware L2 cache flush (active-low). */
		mmio_clr_32(CLUSTER_CTRL_REG0, BIT(4));
		/* Put the cluster back into coherency (deassert ACINACTM). */
		mmio_clr_32(CLUSTER_CTRL_REG1, BIT(0));
		/*
		 * Deassert all cluster resets (active-low). From MSB to LSB:
		 *     Bit 28: AXI2MBUS interface reset
		 *     Bit 24: SoC debug reset
		 *     Bit 20: CPU MBIST reset
		 *     Bit 12: Cluster H_RESET
		 *     Bit  8: L2 cache reset
		 */
		mmio_write_32(CLUSTER_RESET_CTRL_REG,
		              BIT(28) | BIT(24) | BIT(20) | BIT(12) | BIT(8));
		/* Restore the reset vector base addresses for all cores. */
		for (uint32_t i = 0; i < css_get_core_count(cluster); ++i)
			mmio_write_32(RVBA_LO_REG(i), rvba);
	} else if (state == SCPI_CSS_OFF) {
		/* Wait for all CPUs to be idle. */
		mmio_poll_32(CPU_STATUS_REG, GENMASK(19, 16));
		/* Save the power-on reset vector base address from core 0. */
		rvba = mmio_read_32(RVBA_LO_REG(0));
		/* Assert L2FLUSHREQ to clean the cluster L2 cache. */
		mmio_set_32(GENERAL_CTRL_REG0, BIT(8));
		/* Wait for L2FLUSHDONE to go high. */
		mmio_poll_32(L2_STATUS_REG, BIT(10));
		/* Deassert L2FLUSHREQ. */
		mmio_clr_32(GENERAL_CTRL_REG0, BIT(8));
		/* Remove the cluster from coherency (assert ACINACTM). */
		mmio_set_32(CLUSTER_CTRL_REG1, BIT(0));
		/* Wait for the cluster (L2 cache) to be idle. */
		mmio_poll_32(CPU_STATUS_REG, BIT(0));
		/* Assert all cluster resets (active-low). */
		mmio_write_32(CLUSTER_RESET_CTRL_REG, 0);
		/* Assert an undocumented reset bit (active-low). */
		mmio_write_32(CPU_SYS_RESET_REG, 0);
		udelay(1);
		/* Activate the cluster output clamps. */
		mmio_set_32(CLUSTER_PWROFF_GATING_REG, BIT(0));
		/* Remove power from the cluster power domain. */
		css_set_power_switch(CPU_PWR_CLAMP_REG(0), false);
	} else {
		return SCPI_E_PARAM;
	}

	return SCPI_OK;
}

int
css_set_core_state(uint32_t cluster UNUSED, uint32_t core, uint32_t state)
{
	if (state == SCPI_CSS_ON) {
		/* Deassert DBGPWRDUP (prevent debug access to the core). */
		mmio_clr_32(DEBUG_REG0, BIT(core));
		/* Assert core reset (active-low). */
		mmio_clr_32(CLUSTER_RESET_CTRL_REG, BIT(core));
		/* Assert core power-on reset (active-low). */
		mmio_clr_32(CLUSTER_PWRON_RESET_REG, BIT(core));
		/* Program the core to start in AArch64 mode. */
		mmio_set_32(CLUSTER_CTRL_REG0, BIT(24 + core));
		/* Core 0 does not have a separate power domain. */
		if (core > 0) {
			/* Turn on power to the core power domain. */
			css_set_power_switch(CPU_PWR_CLAMP_REG(core), true);
			/* Release the core output clamps. */
			mmio_clr_32(CLUSTER_PWROFF_GATING_REG, BIT(core));
		}
		/* Deassert core power-on reset (active-low). */
		mmio_set_32(CLUSTER_PWRON_RESET_REG, BIT(core));
		/* Deassert core reset (active-low). */
		mmio_set_32(CLUSTER_RESET_CTRL_REG, BIT(core));
		/* Assert DBGPWRDUP (allow debug access to the core). */
		mmio_set_32(DEBUG_REG0, BIT(core));
	} else if (state == SCPI_CSS_OFF) {
		/* Wait for the core to be in WFI and ready to shut down. */
		mmio_poll_32(CPU_STATUS_REG, BIT(16 + core));
		/* Deassert DBGPWRDUP (prevent debug access to the core). */
		mmio_clr_32(DEBUG_REG0, BIT(core));
		/* Core 0 does not have a separate power domain. */
		if (core > 0) {
			/* Activate the core output clamps. */
			mmio_set_32(CLUSTER_PWROFF_GATING_REG, BIT(core));
		}
		/* Assert core reset (active-low). */
		mmio_clr_32(CLUSTER_RESET_CTRL_REG, BIT(core));
		/* Assert core power-on reset (active-low). */
		mmio_clr_32(CLUSTER_PWRON_RESET_REG, BIT(core));
		/* Core 0 does not have a separate power domain. */
		if (core > 0) {
			/* Remove power from the core power domain. */
			css_set_power_switch(CPU_PWR_CLAMP_REG(core), false);
		}
	} else {
		/* Unknown power state requested. */
		return SCPI_E_PARAM;
	}

	return SCPI_OK;
}
