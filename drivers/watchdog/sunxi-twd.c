/*
 * Copyright © 2017-2020 The Crust Firmware Authors.
 * SPDX-License-Identifier: BSD-3-Clause OR GPL-2.0-only
 */

#include <error.h>
#include <mmio.h>
#include <util.h>
#include <clock/ccu.h>
#include <watchdog/sunxi-twd.h>
#include <platform/devices.h>
#include <platform/time.h>

#include "watchdog.h"

#define TWD_STATUS_REG  0x00
#define TWD_CTRL_REG    0x10
#define TWD_RESTART_REG 0x14
#define TWD_INTV_REG    0x30

#define TWD_RESTART_KEY (0xD14 << 16)

#define TWD_TIMEOUT     (5 * REFCLK_HZ) /* 5 seconds */

static void
sunxi_twd_restart(const struct device *dev)
{
	const struct simple_device *self = to_simple_device(dev);

	mmio_write_32(self->regs + TWD_RESTART_REG, TWD_RESTART_KEY | BIT(0));
}

static void
sunxi_twd_set_timeout(const struct device *dev, uint32_t timeout)
{
	const struct simple_device *self = to_simple_device(dev);

	mmio_write_32(self->regs + TWD_INTV_REG, timeout);
}

static int
sunxi_twd_probe(const struct device *dev)
{
	const struct simple_device *self = to_simple_device(dev);
	uintptr_t regs = self->regs;
	int err;

	if ((err = simple_device_probe(dev)))
		return err;

	/* Clear the watchdog configuration. */
	mmio_write_32(regs + TWD_CTRL_REG, BIT(0));
	mmio_pollz_32(regs + TWD_CTRL_REG, BIT(0));

	/* Set counter clock source to OSC24M. */
	mmio_set_32(regs + TWD_CTRL_REG, BIT(31));

	/* Program a conservative default timeout. */
	mmio_write_32(regs + TWD_INTV_REG, TWD_TIMEOUT);

	/* Update the comparator to (counter + timeout). */
	sunxi_twd_restart(dev);

	/* Start the watchdog counter; enable system reset. */
	mmio_clrset_32(regs + TWD_CTRL_REG, BIT(1), BIT(9));

	return SUCCESS;
}

static void
sunxi_twd_release(const struct device *dev)
{
	const struct simple_device *self = to_simple_device(dev);

	/* Disable system reset; stop the watchdog counter. */
	mmio_clrset_32(self->regs + TWD_CTRL_REG, BIT(9), BIT(1));

	simple_device_release(dev);
}

static const struct watchdog_driver sunxi_twd_driver = {
	.drv = {
		.probe   = sunxi_twd_probe,
		.release = sunxi_twd_release,
	},
	.ops = {
		.restart     = sunxi_twd_restart,
		.set_timeout = sunxi_twd_set_timeout,
	},
};

const struct simple_device r_twd = {
	.dev = {
		.name  = "r_twd",
		.drv   = &sunxi_twd_driver.drv,
		.state = DEVICE_STATE_INIT,
	},
	.clock = { .dev = &r_ccu.dev, .id = CLK_BUS_R_TWD },
	.regs  = DEV_R_TWD,
};
