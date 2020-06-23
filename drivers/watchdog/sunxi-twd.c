/*
 * Copyright © 2017-2020 The Crust Firmware Authors.
 * SPDX-License-Identifier: BSD-3-Clause OR GPL-2.0-only
 */

#include <error.h>
#include <intrusive.h>
#include <mmio.h>
#include <util.h>
#include <watchdog.h>
#include <clock/ccu.h>
#include <watchdog/sunxi-twd.h>
#include <platform/devices.h>

#include "watchdog.h"

#define TWD_STATUS_REG  0x00
#define TWD_CTRL_REG    0x10
#define TWD_RESTART_REG 0x14
#define TWD_INTV_REG    0x30

#define TWD_RESTART_KEY (0xD14 << 16)

static inline const struct sunxi_twd *
to_sunxi_twd(const struct device *dev)
{
	return container_of(dev, const struct sunxi_twd, dev);
}

static void
sunxi_twd_restart(const struct device *dev)
{
	const struct sunxi_twd *self = to_sunxi_twd(dev);

	mmio_write_32(self->regs + TWD_RESTART_REG, TWD_RESTART_KEY | BIT(0));
}

static void
sunxi_twd_disable(const struct device *dev)
{
	const struct sunxi_twd *self = to_sunxi_twd(dev);

	/* Disable system reset; stop the watchdog counter. */
	mmio_clrset_32(self->regs + TWD_CTRL_REG, BIT(9), BIT(1));
}

static int
sunxi_twd_enable(const struct device *dev, uint32_t timeout)
{
	const struct sunxi_twd *self = to_sunxi_twd(dev);
	uintptr_t regs = self->regs;

	/* Program the interval until the watchdog fires. */
	mmio_write_32(regs + TWD_INTV_REG, timeout);

	/* Update the comparator from the current counter value. */
	sunxi_twd_restart(dev);

	/* Resume the watchdog counter; enable system reset. */
	mmio_clrset_32(regs + TWD_CTRL_REG, BIT(1), BIT(9));

	return SUCCESS;
}

static int
sunxi_twd_probe(const struct device *dev)
{
	const struct sunxi_twd *self = to_sunxi_twd(dev);
	uintptr_t regs = self->regs;
	int err;

	if ((err = clock_get(&self->clock)))
		return err;

	/* Clear the watchdog configuration. */
	mmio_write_32(regs + TWD_CTRL_REG, BIT(0));
	mmio_pollz_32(regs + TWD_CTRL_REG, BIT(0));

	/* Set counter clock source to OSC24M. */
	mmio_set_32(regs + TWD_CTRL_REG, BIT(31));

	return SUCCESS;
}

static void
sunxi_twd_release(const struct device *dev)
{
	const struct sunxi_twd *self = to_sunxi_twd(dev);

	sunxi_twd_disable(dev);
	clock_put(&self->clock);
}

static const struct watchdog_driver sunxi_twd_driver = {
	.drv = {
		.probe   = sunxi_twd_probe,
		.release = sunxi_twd_release,
	},
	.ops = {
		.disable = sunxi_twd_disable,
		.enable  = sunxi_twd_enable,
		.restart = sunxi_twd_restart,
	},
};

const struct sunxi_twd r_twd = {
	.dev = {
		.name  = "r_twd",
		.drv   = &sunxi_twd_driver.drv,
		.state = DEVICE_STATE_INIT,
	},
	.clock = { .dev = &r_ccu.dev, .id = CLK_BUS_R_TWD },
	.regs  = DEV_R_TWD,
};
