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

static struct sunxi_ccu_clock *
get_clock(struct device *dev, uint8_t id)
{
	return &((struct sunxi_ccu_clock *)(dev->drvdata))[id];
}

static int
sunxi_ccu_disable_id(struct device *dev, int id)
{
	struct sunxi_ccu_clock *clock = get_clock(dev, id);
	uint16_t gate  = clock->gate;
	uint16_t reset = clock->reset;

	/* Put the device in reset before turning off its clock. */
	if (reset != 0) {
		bitmap_clear(dev->regs, reset);
		if (bitmap_get(dev->regs, reset))
			return EIO;
	}
	if (gate != 0) {
		bitmap_clear(dev->regs, gate);
		if (bitmap_get(dev->regs, gate))
			return EIO;
	}

	return SUCCESS;
}

static int
sunxi_ccu_enable_id(struct device *dev, int id)
{
	struct sunxi_ccu_clock *clock = get_clock(dev, id);
	uint16_t gate  = clock->gate;
	uint16_t reset = clock->reset;

	/* Enable the clock before taking the device out of reset. */
	if (gate != 0) {
		bitmap_set(dev->regs, gate);
		if (!bitmap_get(dev->regs, gate))
			return EIO;
	}
	/* Only deassert reset once the device has a running clock. */
	if (reset != 0) {
		bitmap_set(dev->regs, reset);
		if (!bitmap_get(dev->regs, reset))
			return EIO;
	}

	return SUCCESS;
}

static int
sunxi_ccu_disable(struct device *clockdev, struct device *dev)
{
	return sunxi_ccu_disable_id(clockdev, dev->clock);
}

static int
sunxi_ccu_enable(struct device *clockdev, struct device *dev)
{
	return sunxi_ccu_enable_id(clockdev, dev->clock);
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
	assert(dev->drvdata);

	return SUCCESS;
}

const struct driver sunxi_ccu_driver = {
	.name  = "sunxi-ccu",
	.class = DM_CLASS_CLOCK,
	.probe = sunxi_ccu_probe,
	.ops   = &sunxi_ccu_driver_ops,
};
