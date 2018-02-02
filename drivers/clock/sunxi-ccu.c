/*
 * Copyright © 2017-2018 The Crust Firmware Authors.
 * SPDX-License-Identifier: (BSD-3-Clause OR GPL-2.0)
 */

#include <bitmap.h>
#include <compiler.h>
#include <dm.h>
#include <error.h>
#include <drivers/clock.h>
#include <drivers/clock/sunxi-ccu.h>
#include <platform/ccu.h>

static int
sunxi_ccu_probe(struct device *dev __unused)
{
	return SUCCESS;
}

static int
sunxi_ccu_disable(struct device *clockdev, struct device *dev)
{
	uint8_t gate;
	uint8_t reset;

	/* Put the device in reset before turning off its clock. */
	if ((reset = CCU_GET_RESET(dev->clock))) {
		bitmap_clear(clockdev->regs + CCU_RESET_BASE, reset);
		if (bitmap_get(clockdev->regs + CCU_RESET_BASE, reset))
			return EIO;
	}
	if ((gate = CCU_GET_GATE(dev->clock))) {
		bitmap_clear(clockdev->regs + CCU_GATE_BASE, gate);
		if (bitmap_get(clockdev->regs + CCU_GATE_BASE, gate))
			return EIO;
	}

	return SUCCESS;
}

static int
sunxi_ccu_enable(struct device *clockdev, struct device *dev)
{
	uint8_t gate;
	uint8_t reset;

	/* Enable the clock before taking the device out of reset. */
	if ((gate = CCU_GET_GATE(dev->clock))) {
		bitmap_set(clockdev->regs + CCU_GATE_BASE, gate);
		if (!bitmap_get(clockdev->regs + CCU_GATE_BASE, gate))
			return EIO;
	}
	if ((reset = CCU_GET_RESET(dev->clock))) {
		bitmap_set(clockdev->regs + CCU_RESET_BASE, reset);
		if (!bitmap_get(clockdev->regs + CCU_RESET_BASE, reset))
			return EIO;
	}

	return SUCCESS;
}

static int
sunxi_ccu_set_freq(struct device *clockdev __unused,
                   struct device *dev __unused, uint32_t hz __unused)
{
	return ENOTSUP;
}

static const struct clock_driver_ops sunxi_ccu_driver_ops = {
	.disable  = sunxi_ccu_disable,
	.enable   = sunxi_ccu_enable,
	.set_freq = sunxi_ccu_set_freq,
};

const struct driver sunxi_ccu_driver = {
	.name  = "sunxi-ccu",
	.class = DM_CLASS_CLOCK,
	.probe = sunxi_ccu_probe,
	.ops   = &sunxi_ccu_driver_ops,
};
