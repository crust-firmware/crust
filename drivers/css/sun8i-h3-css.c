/*
 * Copyright © 2017-2021 The Crust Firmware Authors.
 * SPDX-License-Identifier: BSD-3-Clause OR GPL-2.0-only
 */

#include <mmio.h>
#include <scpi_protocol.h>
#include <stdint.h>
#include <platform/cpucfg.h>
#include <platform/prcm.h>

#include "css.h"

void
css_resume_core(uint32_t cluster UNUSED, uint32_t core, uint32_t old_state)
{
	if (old_state == SCPI_CSS_OFF) {
		/* Assert core reset and power-on reset (active-low). */
		mmio_write_32(CPUn_RST_CTRL_REG(core), 0);
		/* Enable hardware L1 cache flush (active-low). */
		mmio_clr_32(GEN_CTRL_REG, GEN_CTRL_REG_L1RSTDISABLE(core));
		/* Turn on power to the core power domain. */
		css_set_power_switch(C0_CPUn_PWR_SWITCH_REG(core), true);
		/* Release the core output clamps. */
		mmio_clr_32(C0_PWROFF_GATING_REG, C0_CPUn_PWROFF_GATING(core));
		/* Deassert core reset and power-on reset (active-low). */
		mmio_write_32(CPUn_RST_CTRL_REG(core),
		              CPUn_RST_CTRL_REG_nCORERESET |
		              CPUn_RST_CTRL_REG_nCPUPORESET);
		/* Assert DBGPWRDUP (allow debug access to the core). */
		mmio_set_32(DBG_CTRL_REG1, DBG_CTRL_REG1_DBGPWRDUP(core));
	}
}

void
css_suspend_core(uint32_t cluster UNUSED, uint32_t core, uint32_t new_state)
{
	if (new_state == SCPI_CSS_OFF) {
		/* Wait for the core to be in WFI and ready to shut down. */
		mmio_poll_32(CPUn_STATUS_REG(core),
		             CPUn_STATUS_REG_STANDBYWFI);
		/* Deassert DBGPWRDUP (prevent debug access to the core). */
		mmio_clr_32(DBG_CTRL_REG1, DBG_CTRL_REG1_DBGPWRDUP(core));
		/* Assert core reset and power-on reset (active-low). */
		mmio_write_32(CPUn_RST_CTRL_REG(core), 0);
		/* Activate the core output clamps. */
		mmio_set_32(C0_PWROFF_GATING_REG, C0_CPUn_PWROFF_GATING(core));
		/* Remove power from the core power domain. */
		css_set_power_switch(C0_CPUn_PWR_SWITCH_REG(core), false);
	}
}
