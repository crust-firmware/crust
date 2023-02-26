/*
 * Copyright © 2017-2022 The Crust Firmware Authors.
 * SPDX-License-Identifier: BSD-3-Clause OR GPL-2.0-only
 */

#include <mmio.h>
#include <scpi_protocol.h>
#include <stdint.h>
#include <clock/ccu.h>
#include <platform/cpucfg.h>
#include <platform/devices.h>
#include <platform/prcm.h>

#include "css.h"

#define PLL_PERIPH0_CTRL_REG 0x0028
#define CPUX_AXI_CFG_REG     0x0050
#define AHB1_APB1_CFG_REG    0x0054

#if CONFIG(PLATFORM_H3)

uint32_t
css_get_irq_status(void)
{
	return mmio_read_32(IRQ_FIQ_STATUS_REG);
}

#endif

void
css_suspend_css(uint32_t new_state)
{
	if (new_state < SCPI_CSS_OFF)
		return;

	/* Assert the CPU subsystem reset (active-low). */
	mmio_write_32(CPU_SYS_RESET_REG, 0);
}

void
css_resume_css(uint32_t old_state)
{
	if (old_state < SCPI_CSS_OFF)
		return;

	/* Deassert the CPU subsystem reset (active-low). */
	mmio_write_32(CPU_SYS_RESET_REG, CPU_SYS_RESET);
}

void
css_suspend_cluster(uint32_t cluster, uint32_t new_state)
{
	if (new_state < SCPI_CSS_RETENTION)
		return;

	/* Lower the cluster clock frequency. */
	ccu_suspend_cluster(cluster);
}

void
css_resume_cluster(uint32_t cluster, uint32_t old_state)
{
	if (old_state < SCPI_CSS_RETENTION)
		return;

	/* Raise the cluster clock frequency. */
	ccu_resume_cluster(cluster);
}

void
css_suspend_core(uint32_t cluster UNUSED, uint32_t core, uint32_t new_state)
{
	if (new_state < SCPI_CSS_OFF)
		return;

	/* Wait for the core to be in WFI and ready to shut down. */
	mmio_poll_32(CPUn_STATUS_REG(core), CPUn_STATUS_REG_STANDBYWFI);
	/* Deassert DBGPWRDUP (prevent debug access to the core). */
	mmio_clr_32(DBG_CTRL_REG1, DBG_CTRL_REG1_DBGPWRDUP(core));
	/* Core 0 does not have a separate power domain. */
	if (core > 0) {
		/* Activate the core output clamps. */
		mmio_set_32(C0_PWROFF_GATING_REG, C0_CPUn_PWROFF_GATING(core));
		/* Remove power from the core power domain. */
		css_set_power_switch(C0_CPUn_PWR_SWITCH_REG(core), false);
	} else {
		/* Assert core reset and power-on reset (active-low). */
		mmio_write_32(CPUn_RST_CTRL_REG(core), 0);
	}
}

void
css_resume_core(uint32_t cluster UNUSED, uint32_t core, uint32_t old_state)
{
	uint32_t bus_clk, cpu_clk;

	if (old_state < SCPI_CSS_OFF)
		return;

	/* Core 0 does not have a separate power domain. */
	if (core > 0) {
		/* Assert core reset and power-on reset (active-low). */
		mmio_write_32(CPUn_RST_CTRL_REG(core), 0);
		/* Turn on power to the core power domain. */
		css_set_power_switch(C0_CPUn_PWR_SWITCH_REG(core), true);
		/* Release the core output clamps. */
		mmio_clr_32(C0_PWROFF_GATING_REG, C0_CPUn_PWROFF_GATING(core));
	} else if (CONFIG(PLATFORM_H3)) {
		/* Save registers that will be clobbered by the BROM. */
		cpu_clk = mmio_read_32(DEV_CCU + CPUX_AXI_CFG_REG);
		bus_clk = mmio_read_32(DEV_CCU + AHB1_APB1_CFG_REG);

		/* Bypass PLL_PERIPH0 so AHB1 frequency does not spike. */
		mmio_set_32(DEV_CCU + PLL_PERIPH0_CTRL_REG, BIT(25));
	}
	/* Deassert core reset and power-on reset (active-low). */
	mmio_write_32(CPUn_RST_CTRL_REG(core),
	              CPUn_RST_CTRL_REG_nCORERESET |
	              CPUn_RST_CTRL_REG_nCPUPORESET);
	if (core == 0 && CONFIG(PLATFORM_H3)) {
		/* Spin until the BROM has clobbered the clock registers. */
		mmio_pollz_32(DEV_CCU + AHB1_APB1_CFG_REG, BIT(13));

		/* Disable PLL_PERIPH0 bypass. */
		mmio_clr_32(DEV_CCU + PLL_PERIPH0_CTRL_REG, BIT(25));

		/* Restore the clobbered registers. */
		mmio_write_32(DEV_CCU + CPUX_AXI_CFG_REG, cpu_clk);
		mmio_write_32(DEV_CCU + AHB1_APB1_CFG_REG, bus_clk);
	}
	/* Assert DBGPWRDUP (allow debug access to the core). */
	mmio_set_32(DBG_CTRL_REG1, DBG_CTRL_REG1_DBGPWRDUP(core));
}

void
css_init(void)
{
	/* Enable hardware L1/L2 cache flush for all cores (active-low). */
	mmio_clr_32(GEN_CTRL_REG,
	            GEN_CTRL_REG_L2RSTDISABLE |
	            GEN_CTRL_REG_L1RSTDISABLE_MASK);
}
