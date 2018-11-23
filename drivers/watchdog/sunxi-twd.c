/*
 * Copyright © 2017-2018 The Crust Firmware Authors.
 * SPDX-License-Identifier: BSD-3-Clause OR GPL-2.0-only
 */

#include <error.h>
#include <mmio.h>
#include <timer.h>
#include <watchdog.h>
#include <watchdog/sunxi-twd.h>

#define TWD_STATUS_REG  0x00
#define TWD_CTRL_REG    0x10
#define TWD_RESTART_REG 0x14
#define TWD_INTV_REG    0x30

#define TWD_RESTART_KEY (0xD14 << 16)

static void
sunxi_twd_restart(void *param)
{
	struct device *dev = param;

	/* Enable and perform restart. */
	mmio_write32(dev->regs + TWD_RESTART_REG, TWD_RESTART_KEY | BIT(0));
}

static int
sunxi_twd_disable(struct device *dev)
{
	/* Disable system reset, stop watchdog counter. */
	mmio_clearsetbits32(dev->regs + TWD_CTRL_REG, BIT(9), BIT(1));

	/* Cancel TWD restart timer. */
	return timer_cancel_periodic(sunxi_twd_restart, dev);
}

static int
sunxi_twd_enable(struct device *dev, uint32_t timeout)
{
	/* Program interval until watchdog fires. */
	mmio_write32(dev->regs + TWD_INTV_REG, timeout);

	/* Resume watchdog counter, enable system reset. */
	mmio_clearsetbits32(dev->regs + TWD_CTRL_REG, BIT(1), BIT(9));

	/* Register TWD restart timer. */
	if (!timer_run_periodic(sunxi_twd_restart, dev)) {
		sunxi_twd_restart(dev);
		return SUCCESS;
	}

	return EPERM;
}

static int
sunxi_twd_probe(struct device *dev)
{
	int err;

	if ((err = dm_setup_clocks(dev, 1)))
		return err;

	/* Disable watchdog. */
	sunxi_twd_disable(dev);

	/* Set counter clock source to OSC24M. */
	mmio_setbits32(dev->regs + TWD_CTRL_REG, BIT(31));

	return SUCCESS;
}

const struct watchdog_driver sunxi_twd_driver = {
	.drv = {
		.class = DM_CLASS_WATCHDOG,
		.probe = sunxi_twd_probe,
	},
	.ops = {
		.disable = sunxi_twd_disable,
		.enable  = sunxi_twd_enable,
	},
};
