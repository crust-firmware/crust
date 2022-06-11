/*
 * Copyright © 2017-2022 The Crust Firmware Authors.
 * SPDX-License-Identifier: BSD-3-Clause OR GPL-2.0-only
 */

#include <clock.h>
#include <delay.h>
#include <mmio.h>
#include <clock/ccu.h>
#include <platform/devices.h>
#include <platform/prcm.h>

#define MC_MAER0              (DEV_DRAMCOM + 0x0020)
#define MC_MAER0_VALUE        0xffffffff
#define MC_MAER1              (DEV_DRAMCOM + 0x0024)
#define MC_MAER1_VALUE        0x000007ff
#define MC_MAER2              (DEV_DRAMCOM + 0x0028)
#define MC_MAER2_VALUE        0x0000ffff

#define STATR                 (DEV_DRAMCTL + 0x0004)
#define STATR_OP_MODE         (0x7 << 0)
#define STATR_OP_MODE_NORMAL  (0x1 << 0)
#define STATR_OP_MODE_SELFREF (0x3 << 0)

#define CLKEN                 (DEV_DRAMCTL + 0x000c)
#define CLKEN_VALUE           0x00008100

#define PWRCTL                (DEV_DRAMCTL + 0x0030)
#define PWRCTL_SELFREF_EN     (0x1 << 0)

#define PGCR3                 (DEV_DRAMPHY + 0x001c)
#define PGCR3_CKEN            (0xf << 16)
#define PGCR3_CKEN_DISABLED   (0x0 << 16)
#define PGCR3_CKEN_INVERTED   (0x5 << 16)
#define PGCR3_CKEN_NORMAL     (0xa << 16)
#define PGCR3_CKNEN           (0xf << 20)
#define PGCR3_CKNEN_DISABLED  (0x0 << 20)
#define PGCR3_CKNEN_INVERTED  (0x5 << 20)
#define PGCR3_CKNEN_NORMAL    (0xa << 20)

/* Clocks needed by this driver. */
enum {
	BUS_DRAM,
	DRAM,
	MBUS,
};

static const struct clock_handle dram_clocks[] = {
	[BUS_DRAM] = {
		.dev = &ccu.dev,
		.id  = CLK_BUS_DRAM,
	},
	[DRAM] = {
		.dev = &ccu.dev,
		.id  = CLK_DRAM,
	},
	[MBUS] = {
		.dev = &ccu.dev,
		.id  = CLK_MBUS,
	},
};

void
dram_suspend(void)
{
	/* Enable DRAM controller register access. */
	clock_get(&dram_clocks[BUS_DRAM]);

	/* Disable all controller masters. */
	mmio_write_32(MC_MAER0, 0);
	mmio_write_32(MC_MAER1, 0);
	mmio_write_32(MC_MAER2, 0);
	/* Enable DRAM self-refresh. */
	mmio_set_32(PWRCTL, PWRCTL_SELFREF_EN);
	/* Wait until the DRAM controller enters self-refresh. */
	mmio_polleq_32(STATR, STATR_OP_MODE, STATR_OP_MODE_SELFREF);

	/* Disable CKEN and CKNEN. */
	mmio_clrset_32(PGCR3,
	               PGCR3_CKEN |
	               PGCR3_CKNEN,
	               PGCR3_CKEN_DISABLED |
	               PGCR3_CKNEN_DISABLED);

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

	/* Enable CKEN and CKNEN. */
	mmio_clrset_32(PGCR3,
	               PGCR3_CKEN |
	               PGCR3_CKNEN,
	               PGCR3_CKEN_NORMAL |
	               PGCR3_CKNEN_NORMAL);

	/* Disable DRAM self refresh. */
	mmio_clr_32(PWRCTL, PWRCTL_SELFREF_EN);
	/* Wait until the DRAM controller exits self-refresh. */
	mmio_polleq_32(STATR, STATR_OP_MODE, STATR_OP_MODE_NORMAL);
	/* Enable all controller masters. */
	mmio_write_32(MC_MAER0, MC_MAER0_VALUE);
	mmio_write_32(MC_MAER1, MC_MAER1_VALUE);
	mmio_write_32(MC_MAER2, MC_MAER2_VALUE);

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
