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

static int
sunxi_ccu_disable(struct device *clockdev, struct device *dev)
{
	uint16_t gate;
	uint16_t reset;

	/* Put the device in reset before turning off its clock. */
	if ((reset = CCU_GET_RESET(dev->clock))) {
		bitmap_clear(clockdev->regs, reset);
		if (bitmap_get(clockdev->regs, reset))
			return EIO;
	}
	if ((gate = CCU_GET_GATE(dev->clock))) {
		bitmap_clear(clockdev->regs, gate);
		if (bitmap_get(clockdev->regs, gate))
			return EIO;
	}

	return SUCCESS;
}

static int
sunxi_ccu_enable(struct device *clockdev, struct device *dev)
{
	uint16_t gate;
	uint16_t reset;

	/* Enable the clock before taking the device out of reset. */
	if ((gate = CCU_GET_GATE(dev->clock))) {
		bitmap_set(clockdev->regs, gate);
		if (!bitmap_get(clockdev->regs, gate))
			return EIO;
	}
	if ((reset = CCU_GET_RESET(dev->clock))) {
		bitmap_set(clockdev->regs, reset);
		if (!bitmap_get(clockdev->regs, reset))
			return EIO;
	}

	return SUCCESS;
}

static int
sunxi_ccu_get_rate(struct device *clockdev __unused,
                   struct device *dev __unused, uint32_t *rate __unused)
{
	return ENOTSUP;
}

static int
sunxi_ccu_set_rate(struct device *clockdev __unused,
                   struct device *dev __unused, uint32_t rate __unused)
{
	return ENOTSUP;
}

static const struct clock_driver_ops sunxi_ccu_driver_ops = {
	.disable  = sunxi_ccu_disable,
	.enable   = sunxi_ccu_enable,
	.get_rate = sunxi_ccu_get_rate,
	.set_rate = sunxi_ccu_set_rate,
};

static int
sunxi_ccu_probe(struct device *dev __unused)
{
	return SUCCESS;
}

const struct driver sunxi_ccu_driver = {
	.name  = "sunxi-ccu",
	.class = DM_CLASS_CLOCK,
	.probe = sunxi_ccu_probe,
	.ops   = &sunxi_ccu_driver_ops,
};
