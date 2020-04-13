/*
 * Copyright © 2017-2020 The Crust Firmware Authors.
 * SPDX-License-Identifier: BSD-3-Clause OR GPL-2.0-only
 */

#include <clock.h>
#include <css.h>
#include <debug.h>
#include <delay.h>
#include <mmio.h>
#include <scpi_protocol.h>
#include <stdbool.h>
#include <stdint.h>
#include <util.h>
#include <clock/ccu.h>
#include <platform/devices.h>

#include "css.h"

#define CLUSTER_MAX               1
#define CORE_MAX                  4

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

/* mctl_reg-sun8iw11.h:63 */
#define MC_MAER                   (DEV_DRAMCOM + 0x0094)

/* mctl_reg-sun8iw11.h:102 */
#define PWRCTL                    (DEV_DRAMCTL + 0x0004)
/* mctl_standby-sun8iw11.c:120,878,891 */
#define PWRCTL_SELFREF_EN         (0x1 << 0)
/* mctl_standby-sun8iw11.c:121 */
#define PWRCTL_UNK08              (0x1 << 8)

/* mctl_reg-sun8iw11.h:104 */
#define CLKEN                     (DEV_DRAMCTL + 0x000c)
/* dram_sunxi_dw.c:410 */
#define CLKEN_VALUE               0xc00e

/* mctl_reg-sun8iw11.h:107 */
#define STATR                     (DEV_DRAMCTL + 0x0018)
/* mctl_standby-sun8iw11.c:124 */
#define STATR_OP_MODE             (0x7 << 0)
#define STATR_OP_MODE_NORMAL      (0x1 << 0)
#define STATR_OP_MODE_SELFREF     (0x3 << 0)

/* mctl_reg-sun8iw11.h:141 */
#define PGCR3                     (DEV_DRAMCTL + 0x010c)
/* mctl_standby-sun8iw11.c:130 */
#define PGCR3_CKEN                (0xf << 16)
#define PGCR3_CKEN_DISABLED       (0x0 << 16)
#define PGCR3_CKEN_INVERTED       (0x5 << 16)
#define PGCR3_CKEN_NORMAL         (0xa << 16)
/* mctl_standby-sun8iw11.c:130 */
#define PGCR3_CKNEN               (0xf << 20)
#define PGCR3_CKNEN_DISABLED      (0x0 << 20)
#define PGCR3_CKNEN_INVERTED      (0x5 << 20)
#define PGCR3_CKNEN_NORMAL        (0xa << 20)
/* mctl_standby-sun8iw11.c:872-873,904-905,950 */
#define PGCR3_UNK25               (0x3 << 25)

/* mctl_reg-sun8iw11.h:146 */
#define ZQCR                      (DEV_DRAMCTL + 0x0140)
#define ZQCR_ZQPD                 (0x1 << 31)
#define ZQCR_ZQPD_DISABLED        (0x0 << 31)
#define ZQCR_ZQPD_ENABLED         (0x1 << 31)

/* mctl_reg-sun8iw11.h:160 */
#define ACIOCR0                   (DEV_DRAMCTL + 0x0208)
/* mctl_standby-sun8iw11.c:154 */
#define ACIOCR0_ACPDD             (0x1 << 0)
#define ACIOCR0_ACPDD_DISABLED    (0x0 << 0)
#define ACIOCR0_ACPDD_ENABLED     (0x1 << 0)
/* mctl_standby-sun8iw11.c:154,762 */
#define ACIOCR0_ACPDR             (0x1 << 1)
#define ACIOCR0_ACPDR_DISABLED    (0x0 << 1)
#define ACIOCR0_ACPDR_ENABLED     (0x1 << 1)
/* mctl_standby-sun8iw11.c:152 */
#define ACIOCR0_ACOE              (0x1 << 3)
#define ACIOCR0_ACOE_DISABLED     (0x0 << 3)
#define ACIOCR0_ACOE_ENABLED      (0x1 << 3)
/* mctl_standby-sun8iw11.c:153 */
#define ACIOCR0_ACIOM             (0x1 << 4)
#define ACIOCR0_ACIOM_SSTL        (0x0 << 4)
#define ACIOCR0_ACIOM_CMOS        (0x1 << 4)
/* mctl_standby-sun8iw11.c:151 */
#define ACIOCR0_CKOE              (0x3 << 6)
#define ACIOCR0_CKOE_DISABLED     (0x0 << 6)
#define ACIOCR0_CKOE_ENABLED      (0x3 << 6)
/* mctl_standby-sun8iw11.c:150 */
#define ACIOCR0_CKEOE             (0x3 << 8)
#define ACIOCR0_CKEOE_DISABLED    (0x0 << 8)
#define ACIOCR0_CKEOE_ENABLED     (0x3 << 8)
/* mctl_standby-sun8iw11.c:763 */
#define ACIOCR0_UNK11             (0x1 << 11)

/* mctl_reg-sun8iw11.h:173 */
#define DXnGCR0(n)                (DEV_DRAMCTL + 0x0344 + 0x80 * (n))
/* mctl_standby-sun8iw11.c:146,710-711 */
#define DXnGCR0_DXEN              (0x1 << 0)
#define DXnGCR0_DXEN_DISABLED     (0x0 << 0)
#define DXnGCR0_DXEN_ENABLED      (0x1 << 0)
/* mctl_standby-sun8iw11.c:145,749 */
#define DXnGCR0_DXIOM             (0x1 << 1)
#define DXnGCR0_DXIOM_SSTL        (0x0 << 1)
#define DXnGCR0_DXIOM_CMOS        (0x1 << 1)
/* mctl_standby-sun8iw11.c:144,750 */
#define DXnGCR0_DXOEO             (0x3 << 2)
#define DXnGCR0_DXOEO_DYNAMIC     (0x0 << 2)
#define DXnGCR0_DXOEO_ENABLED     (0x1 << 2)
#define DXnGCR0_DXOEO_DISABLED    (0x2 << 2)
/* mctl_standby-sun8iw11.c:748 */
#define DXnGCR0_DXODT             (0x3 << 4)
#define DXnGCR0_DXODT_DYNAMIC     (0x0 << 4)
#define DXnGCR0_DXODT_ENABLED     (0x1 << 4)
#define DXnGCR0_DXODT_DISABLED    (0x2 << 4)
/* mctl_standby-sun8iw11.c:754-755 */
#define DXnGCR0_UNK09             (0x3 << 9)
#define DXnGCR0_UNK09_DYNAMIC     (0x0 << 9)
#define DXnGCR0_UNK09_ENABLED     (0x1 << 9)
#define DXnGCR0_UNK09_DISABLED    (0x2 << 9)
/* mctl_standby-sun8iw11.c:143,751 */
#define DXnGCR0_DXPDR             (0x3 << 12)
#define DXnGCR0_DXPDR_DYNAMIC     (0x0 << 12)
#define DXnGCR0_DXPDR_ENABLED     (0x1 << 12)
#define DXnGCR0_DXPDR_DISABLED    (0x2 << 12)
/* mctl_standby-sun8iw11.c:143,752 */
#define DXnGCR0_DXPDD             (0x3 << 14)
#define DXnGCR0_DXPDD_DYNAMIC     (0x0 << 14)
#define DXnGCR0_DXPDD_ENABLED     (0x1 << 14)
#define DXnGCR0_DXPDD_DISABLED    (0x2 << 14)
/* mctl_standby-sun8iw11.c:141 */
#define DXnGCR0_DQSRPD            (0x3 << 22)
#define DXnGCR0_DQSRPD_DYNAMIC    (0x0 << 22)
#define DXnGCR0_DQSRPD_ENABLED    (0x1 << 22)
#define DXnGCR0_DQSRPD_DISABLED   (0x2 << 22)

#define CLUSTER_PWRON_RESET_REG   (DEV_R_CPUCFG + 0x0030)
#define CPU_SYS_RESET_REG         (DEV_R_CPUCFG + 0x0140)

#define CLUSTER_PWROFF_GATING_REG (DEV_R_PRCM + 0x0100)
#define VDD_SYS_PWROFF_GATING_REG (DEV_R_PRCM + 0x0110)
#define CPU_PWR_CLAMP_REG(n)      (DEV_R_PRCM + 0x0140 + 0x04 * (n))

/* Clocks needed by this driver. */
enum {
	BUS_DRAM,
	MBUS,
	DRAM,
	CPUX,
};

static const struct clock_handle css_clocks[] = {
	[BUS_DRAM] = {
		.dev = &ccu.dev,
		.id  = CLK_BUS_DRAM,
	},
	[MBUS] = {
		.dev = &ccu.dev,
		.id  = CLK_MBUS,
	},
	[DRAM] = {
		.dev = &ccu.dev,
		.id  = CLK_DRAM,
	},
	[CPUX] = {
		.dev = &ccu.dev,
		.id  = CLK_CPUX,
	},
};

/* Reset Vector Base Address. */
static uint32_t rvba;

uint8_t
css_get_css_state(void)
{
	/*
	 * The CSS is considered off if the DRAM pads are being held.
	 */
	if (mmio_get_32(VDD_SYS_PWROFF_GATING_REG, GENMASK(1, 0)))
		return SCPI_CSS_OFF;

	return SCPI_CSS_ON;
}

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

void
css_init(void)
{
	/* Get references to clocks that are already running. */
	clock_get(&css_clocks[MBUS]);
	clock_get(&css_clocks[DRAM]);
	clock_get(&css_clocks[CPUX]);
}

int
css_set_css_state(uint8_t state UNUSED)
{
	if (state == css_get_css_state())
		return SCPI_OK;

	/* Enable DRAM controller register access. */
	clock_get(&css_clocks[BUS_DRAM]);

	if (state == SCPI_CSS_ON) {
		/* Enable DRAM controller clocks. */
		clock_get(&css_clocks[MBUS]);
		clock_get(&css_clocks[DRAM]);
		udelay(10);
		mmio_write_32(CLKEN, CLKEN_VALUE);
		udelay(10);
		/* Disable pad hold. */
		mmio_clr_32(VDD_SYS_PWROFF_GATING_REG, GENMASK(1, 0));
		udelay(10);
		/* Configure AC pads. */
		mmio_clrset_32(ACIOCR0,
		               ACIOCR0_ACPDD |
		               ACIOCR0_ACPDR |
		               ACIOCR0_ACOE |
		               ACIOCR0_ACIOM |
		               ACIOCR0_CKOE |
		               ACIOCR0_CKEOE,
		               ACIOCR0_ACPDD_DISABLED |
		               ACIOCR0_ACPDR_ENABLED |
		               ACIOCR0_ACOE_ENABLED |
		               ACIOCR0_ACIOM_SSTL |
		               ACIOCR0_CKOE_ENABLED |
		               ACIOCR0_CKEOE_ENABLED);
		/* Configure DX pads. */
		for (uint8_t n = 0; n < 4; ++n) {
			mmio_clrset_32(DXnGCR0(n),
			               DXnGCR0_DXEN |
			               DXnGCR0_DXIOM |
			               DXnGCR0_DXOEO |
			               DXnGCR0_DXPDR |
			               DXnGCR0_DXPDD |
			               DXnGCR0_DQSRPD,
			               DXnGCR0_DXEN_ENABLED |
			               DXnGCR0_DXIOM_SSTL |
			               DXnGCR0_DXOEO_DYNAMIC |
			               DXnGCR0_DXPDR_DYNAMIC |
			               DXnGCR0_DXPDD_DYNAMIC |
			               DXnGCR0_DQSRPD_DYNAMIC);
		}
		udelay(1);
		/* Enable CKEN and CKNEN. */
		mmio_clrset_32(PGCR3,
		               PGCR3_CKEN |
		               PGCR3_CKNEN,
		               PGCR3_CKEN_NORMAL |
		               PGCR3_CKNEN_NORMAL);
		udelay(1);
		/* Disable DRAM self refresh. */
		mmio_clr_32(PWRCTL, PWRCTL_SELFREF_EN);
		/* Wait until the DRAM controller exits self-refresh. */
		mmio_polleq_32(STATR, STATR_OP_MODE, STATR_OP_MODE_NORMAL);
		udelay(1);
		/* Enable all controller masters. */
		mmio_write_32(MC_MAER, ~0);
	} else if (state == SCPI_CSS_OFF) {
		/* Disable all controller masters. */
		mmio_write_32(MC_MAER, 0);
		/* Enable DRAM self-refresh. */
		mmio_set_32(PWRCTL, PWRCTL_SELFREF_EN);
		/* Wait until the DRAM controller enters self-refresh. */
		mmio_polleq_32(STATR, STATR_OP_MODE, STATR_OP_MODE_SELFREF);
		udelay(1);
		/* Disable CKEN and CKNEN. */
		mmio_clrset_32(PGCR3,
		               PGCR3_CKEN |
		               PGCR3_CKNEN,
		               PGCR3_CKEN_DISABLED |
		               PGCR3_CKNEN_DISABLED);
		udelay(1);
		/* Configure DX pads. */
		for (uint8_t n = 0; n < 4; ++n) {
			mmio_clrset_32(DXnGCR0(n),
			               DXnGCR0_DXEN |
			               DXnGCR0_DXIOM |
			               DXnGCR0_DXOEO |
			               DXnGCR0_DXPDR |
			               DXnGCR0_DXPDD |
			               DXnGCR0_DQSRPD,
			               DXnGCR0_DXEN_DISABLED |
			               DXnGCR0_DXIOM_CMOS |
			               DXnGCR0_DXOEO_DISABLED |
			               DXnGCR0_DXPDR_ENABLED |
			               DXnGCR0_DXPDD_ENABLED |
			               DXnGCR0_DQSRPD_ENABLED);
		}
		/* Configure AC pads. */
		mmio_clrset_32(ACIOCR0,
		               ACIOCR0_ACPDD |
		               ACIOCR0_ACPDR |
		               ACIOCR0_ACOE |
		               ACIOCR0_ACIOM |
		               ACIOCR0_CKOE |
		               ACIOCR0_CKEOE,
		               ACIOCR0_ACPDD_ENABLED |
		               ACIOCR0_ACPDR_ENABLED |
		               ACIOCR0_ACOE_DISABLED |
		               ACIOCR0_ACIOM_CMOS |
		               ACIOCR0_CKOE_DISABLED |
		               ACIOCR0_CKEOE_ENABLED);
		/* Enable pad hold. */
		mmio_set_32(VDD_SYS_PWROFF_GATING_REG, GENMASK(1, 0));
		udelay(10);
		/* Disable DRAM controller clocks. */
		mmio_write_32(CLKEN, 0);
		clock_put(&css_clocks[DRAM]);
		clock_put(&css_clocks[MBUS]);
	} else {
		return SCPI_E_PARAM;
	}

	/* Disable further DRAM controller register access. */
	clock_put(&css_clocks[BUS_DRAM]);

	return SCPI_OK;
}

int
css_set_cluster_state(uint8_t cluster, uint8_t state)
{
	assert(cluster < CLUSTER_MAX);

	if (state == css_get_cluster_state(cluster))
		return SCPI_OK;

	if (state == SCPI_CSS_ON) {
		/* Enable the CPUX clock. */
		clock_get(&css_clocks[CPUX]);
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
		for (uint8_t core = 0; core < CORE_MAX; ++core)
			mmio_write_32(RVBA_LO_REG(core), rvba);
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
		/* Disable the CPUX clock. */
		clock_put(&css_clocks[CPUX]);
	} else {
		return SCPI_E_PARAM;
	}

	return SCPI_OK;
}

int
css_set_core_state(uint8_t cluster, uint8_t core, uint8_t state)
{
	assert(cluster < CLUSTER_MAX);
	assert(core < CORE_MAX);

	if (state == css_get_core_state(cluster, core))
		return SCPI_OK;

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
