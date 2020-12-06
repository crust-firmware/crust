/*
 * Copyright © 2017-2020 The Crust Firmware Authors.
 * SPDX-License-Identifier: BSD-3-Clause OR GPL-2.0-only
 */

#include <clock.h>
#include <delay.h>
#include <mmio.h>
#include <stdint.h>
#include <util.h>
#include <clock/ccu.h>
#include <platform/devices.h>

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

#define VDD_SYS_PWROFF_GATING_REG (DEV_R_PRCM + 0x0110)

/* Clocks needed by this driver. */
enum {
	BUS_DRAM,
	MBUS,
	DRAM,
};

static const struct clock_handle dram_clocks[] = {
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
};

void
dram_suspend(void)
{
	/* Enable DRAM controller register access. */
	clock_get(&dram_clocks[BUS_DRAM]);

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
		               DXnGCR0_DXIOM |
		               DXnGCR0_DXOEO |
		               DXnGCR0_DXPDR |
		               DXnGCR0_DXPDD |
		               DXnGCR0_DQSRPD,
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
	clock_put(&dram_clocks[DRAM]);
	clock_put(&dram_clocks[MBUS]);

	/* Disable further DRAM controller register access. */
	clock_put(&dram_clocks[BUS_DRAM]);
}

void
dram_resume(void)
{
	/* Enable DRAM controller register access. */
	clock_get(&dram_clocks[BUS_DRAM]);

	/* Enable DRAM controller clocks. */
	clock_get(&dram_clocks[MBUS]);
	clock_get(&dram_clocks[DRAM]);
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
		               DXnGCR0_DXIOM |
		               DXnGCR0_DXOEO |
		               DXnGCR0_DXPDR |
		               DXnGCR0_DXPDD |
		               DXnGCR0_DQSRPD,
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

	/* Disable further DRAM controller register access. */
	clock_put(&dram_clocks[BUS_DRAM]);
}

void
dram_init(void)
{
	/* Get references to clocks that are already running. */
	clock_get(&dram_clocks[MBUS]);
	clock_get(&dram_clocks[DRAM]);
}
