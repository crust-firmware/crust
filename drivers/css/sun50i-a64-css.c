/*
 * Copyright © 2017-2020 The Crust Firmware Authors.
 * SPDX-License-Identifier: BSD-3-Clause OR GPL-2.0-only
 */

#include <delay.h>
#include <mmio.h>
#include <scpi_protocol.h>
#include <stdint.h>
#include <platform/cpucfg.h>
#include <platform/prcm.h>

#include "css.h"

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
		css_set_power_switch(C0_CPUn_PWR_SWITCH_REG(0), true);
		/* Release the cluster output clamps. */
		mmio_clr_32(C0_PWROFF_GATING_REG, BIT(0));
		udelay(1);
		/* Deassert an undocumented reset bit (active-low). */
		mmio_set_32(CPU_SYS_RESET_REG, BIT(0));
		udelay(1);
		/* Assert all cluster resets (active-low). */
		mmio_write_32(C0_RST_CTRL_REG, 0);
		/* Enable hardware L2 cache flush (active-low). */
		mmio_clr_32(C0_CTRL_REG0, BIT(4));
		/* Put the cluster back into coherency (deassert ACINACTM). */
		mmio_clr_32(C0_CTRL_REG1, BIT(0));
		/*
		 * Deassert all cluster resets (active-low). From MSB to LSB:
		 *     Bit 28: AXI2MBUS interface reset
		 *     Bit 24: SoC debug reset
		 *     Bit 20: CPU MBIST reset
		 *     Bit 12: Cluster H_RESET
		 *     Bit  8: L2 cache reset
		 */
		mmio_write_32(C0_RST_CTRL_REG,
		              BIT(28) | BIT(24) | BIT(20) | BIT(12) | BIT(8));
		/* Restore the reset vector base addresses for all cores. */
		for (uint32_t i = 0; i < css_get_core_count(cluster); ++i)
			mmio_write_32(RVBA_LO_REG(i), rvba);
	} else if (state == SCPI_CSS_OFF) {
		/* Wait for all CPUs to be idle. */
		mmio_poll_32(C0_CPU_STATUS_REG, GENMASK(19, 16));
		/* Save the power-on reset vector base address from core 0. */
		rvba = mmio_read_32(RVBA_LO_REG(0));
		/* Assert L2FLUSHREQ to clean the cluster L2 cache. */
		mmio_set_32(C0_CTRL_REG2, BIT(8));
		/* Wait for L2FLUSHDONE to go high. */
		mmio_poll_32(L2_STATUS_REG, BIT(10));
		/* Deassert L2FLUSHREQ. */
		mmio_clr_32(C0_CTRL_REG2, BIT(8));
		/* Remove the cluster from coherency (assert ACINACTM). */
		mmio_set_32(C0_CTRL_REG1, BIT(0));
		/* Wait for the cluster (L2 cache) to be idle. */
		mmio_poll_32(C0_CPU_STATUS_REG, BIT(0));
		/* Assert all cluster resets (active-low). */
		mmio_write_32(C0_RST_CTRL_REG, 0);
		/* Assert an undocumented reset bit (active-low). */
		mmio_write_32(CPU_SYS_RESET_REG, 0);
		udelay(1);
		/* Activate the cluster output clamps. */
		mmio_set_32(C0_PWROFF_GATING_REG, BIT(0));
		/* Remove power from the cluster power domain. */
		css_set_power_switch(C0_CPUn_PWR_SWITCH_REG(0), false);
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
		mmio_clr_32(DBG_REG0, BIT(core));
		/* Assert core reset (active-low). */
		mmio_clr_32(C0_RST_CTRL_REG, BIT(core));
		/* Assert core power-on reset (active-low). */
		mmio_clr_32(C0_PWRON_RESET_REG, BIT(core));
		/* Program the core to start in AArch64 mode. */
		mmio_set_32(C0_CTRL_REG0, BIT(24 + core));
		/* Core 0 does not have a separate power domain. */
		if (core > 0) {
			/* Turn on power to the core power domain. */
			css_set_power_switch(C0_CPUn_PWR_SWITCH_REG(core),
			                     true);
			/* Release the core output clamps. */
			mmio_clr_32(C0_PWROFF_GATING_REG, BIT(core));
		}
		/* Deassert core power-on reset (active-low). */
		mmio_set_32(C0_PWRON_RESET_REG, BIT(core));
		/* Deassert core reset (active-low). */
		mmio_set_32(C0_RST_CTRL_REG, BIT(core));
		/* Assert DBGPWRDUP (allow debug access to the core). */
		mmio_set_32(DBG_REG0, BIT(core));
	} else if (state == SCPI_CSS_OFF) {
		/* Wait for the core to be in WFI and ready to shut down. */
		mmio_poll_32(C0_CPU_STATUS_REG, BIT(16 + core));
		/* Deassert DBGPWRDUP (prevent debug access to the core). */
		mmio_clr_32(DBG_REG0, BIT(core));
		/* Core 0 does not have a separate power domain. */
		if (core > 0) {
			/* Activate the core output clamps. */
			mmio_set_32(C0_PWROFF_GATING_REG, BIT(core));
		}
		/* Assert core reset (active-low). */
		mmio_clr_32(C0_RST_CTRL_REG, BIT(core));
		/* Assert core power-on reset (active-low). */
		mmio_clr_32(C0_PWRON_RESET_REG, BIT(core));
		/* Core 0 does not have a separate power domain. */
		if (core > 0) {
			/* Remove power from the core power domain. */
			css_set_power_switch(C0_CPUn_PWR_SWITCH_REG(core),
			                     false);
		}
	} else {
		/* Unknown power state requested. */
		return SCPI_E_PARAM;
	}

	return SCPI_OK;
}
