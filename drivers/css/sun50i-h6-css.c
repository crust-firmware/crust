/*
 * Copyright © 2017-2019 The Crust Firmware Authors.
 * SPDX-License-Identifier: BSD-3-Clause OR GPL-2.0-only
 */

#include <css.h>
#include <debug.h>
#include <delay.h>
#include <error.h>
#include <mmio.h>
#include <scpi_protocol.h>
#include <stdbool.h>
#include <stdint.h>
#include <util.h>
#include <platform/devices.h>

#include "css.h"

#define CLUSTER_MAX               1
#define CORE_MAX                  4

#define CLUSTER_RESET_CTRL_REG    (DEV_CPUCFG + 0x0000)
#define CLUSTER_CTRL_REG0         (DEV_CPUCFG + 0x0010)
#define CLUSTER_CTRL_REG1         (DEV_CPUCFG + 0x0014)
#define CLUSTER_CTRL_REG2         (DEV_CPUCFG + 0x0018)
#define CACHE_CFG_REG0            (DEV_CPUCFG + 0x0024)
#define RVBA_LO_REG(n)            (DEV_CPUCFG + 0x0040 + 0x08 * (n))
#define RVBA_HI_REG(n)            (DEV_CPUCFG + 0x0044 + 0x08 * (n))
#define CPU_STATUS_REG            (DEV_CPUCFG + 0x0080)
#define L2_STATUS_REG             (DEV_CPUCFG + 0x0084)
#define DEBUG_REG0                (DEV_CPUCFG + 0x00c0)
#define DEBUG_REG1                (DEV_CPUCFG + 0x00c4)

#define CLUSTER_PWRON_RESET_REG   (DEV_R_CPUCFG + 0x0040)
#define CLUSTER_PWROFF_GATING_REG (DEV_R_CPUCFG + 0x0044)
#define CPU_PWR_CLAMP_REG(n)      (DEV_R_CPUCFG + 0x0050 + 0x04 * (n))
#define CPU_SYS_RESET_REG         (DEV_R_CPUCFG + 0x00a0)

/* Reset Vector Base Address. */
static uint32_t rvba;

uint8_t
css_get_cluster_count(void)
{
	return CLUSTER_MAX;
}

uint8_t
css_get_cluster_state(uint8_t cluster UNUSED)
{
	assert(cluster < CLUSTER_MAX);

	/*
	 * The cluster is considered off if its L2 cache is in reset.
	 */
	if (!mmio_get_32(CLUSTER_RESET_CTRL_REG, BIT(8)))
		return SCPI_CSS_OFF;

	return SCPI_CSS_ON;
}

uint8_t
css_get_core_count(uint8_t cluster UNUSED)
{
	return CORE_MAX;
}

uint8_t
css_get_core_state(uint8_t cluster UNUSED, uint8_t core)
{
	assert(cluster < CLUSTER_MAX);
	assert(core < CORE_MAX);

	/*
	 * A core is considered off if it is in core reset. Regardless of any
	 * deeper "off" state the core may or may not be in, if it is in
	 * core reset, it is not running instructions or maintaining state.
	 */
	if (!mmio_get_32(CLUSTER_RESET_CTRL_REG, BIT(core)))
		return SCPI_CSS_OFF;

	return SCPI_CSS_ON;
}

int
css_set_css_state(uint8_t state UNUSED)
{
	/* Nothing to do. */
	return SUCCESS;
}

int
css_set_cluster_state(uint8_t cluster, uint8_t state)
{
	assert(cluster < CLUSTER_MAX);

	if (state == css_get_cluster_state(cluster))
		return SUCCESS;

	if (state == SCPI_CSS_ON) {
		/* Put the cluster back into coherency (deassert ACINACTM). */
		mmio_clr_32(CLUSTER_CTRL_REG1, BIT(0));
		/* Restore the reset vector base addresses for all cores. */
		for (uint8_t core = 0; core < CORE_MAX; ++core)
			mmio_write_32(RVBA_LO_REG(core), rvba);
	} else if (state == SCPI_CSS_OFF) {
		/* Wait for all CPUs to be idle. */
		mmio_poll_32(CPU_STATUS_REG, GENMASK(19, 16));
		/* Save the power-on reset vector base address from core 0. */
		rvba = mmio_read_32(RVBA_LO_REG(0));
		/* Assert L2FLUSHREQ to clean the cluster L2 cache. */
		mmio_set_32(CLUSTER_CTRL_REG2, BIT(8));
		/* Wait for L2FLUSHDONE to go high. */
		mmio_poll_32(L2_STATUS_REG, BIT(10));
		/* Deassert L2FLUSHREQ. */
		mmio_clr_32(CLUSTER_CTRL_REG2, BIT(8));
		/* Remove the cluster from coherency (assert ACINACTM). */
		mmio_set_32(CLUSTER_CTRL_REG1, BIT(0));
		/* Wait for the cluster (L2 cache) to be idle. */
		mmio_poll_32(CPU_STATUS_REG, BIT(0));
	} else {
		return EINVAL;
	}

	return SUCCESS;
}

int
css_set_core_state(uint8_t cluster, uint8_t core, uint8_t state)
{
	assert(cluster < CLUSTER_MAX);
	assert(core < CORE_MAX);

	if (state == css_get_core_state(cluster, core))
		return SUCCESS;

	if (state == SCPI_CSS_ON) {
		/* Deassert DBGPWRDUP (prevent debug access to the core). */
		mmio_clr_32(DEBUG_REG0, BIT(core));
		/* Assert core reset (active-low). */
		mmio_clr_32(CLUSTER_RESET_CTRL_REG, BIT(core));
		/* Assert core power-on reset (active-low). */
		mmio_clr_32(CLUSTER_PWRON_RESET_REG, BIT(core));
		/* Program the core to start in AArch64 mode. */
		mmio_set_32(CLUSTER_CTRL_REG0, BIT(24 + core));
		/* Turn on power to the core power domain. */
		css_set_power_switch(CPU_PWR_CLAMP_REG(core), true);
		/* Release the core output clamps. */
		mmio_clr_32(CLUSTER_PWROFF_GATING_REG, BIT(core));
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
		/* Activate the core output clamps. */
		mmio_set_32(CLUSTER_PWROFF_GATING_REG, BIT(core));
		/* Assert core reset (active-low). */
		mmio_clr_32(CLUSTER_RESET_CTRL_REG, BIT(core));
		/* Assert core power-on reset (active-low). */
		mmio_clr_32(CLUSTER_PWRON_RESET_REG, BIT(core));
		/* Remove power from the core power domain. */
		css_set_power_switch(CPU_PWR_CLAMP_REG(core), false);
	} else {
		/* Unknown power state requested. */
		return EINVAL;
	}

	return SUCCESS;
}
