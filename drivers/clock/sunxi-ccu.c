/*
 * Copyright © 2017 Samuel Holland <samuel@sholland.org>
 * SPDX-License-Identifier: (BSD-3-Clause OR GPL-2.0)
 */

#include <compiler.h>
#include <debug.h>
#include <dm.h>
#include <error.h>
#include <mmio.h>
#include <util.h>
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

	if ((reset = CCU_GET_RESET(dev->clock))) {
		mmio_clearbits32(clockdev->regs + CCU_RESET_BASE +
		                 BITMAP_WORDOFF(reset),
		                 BIT(BITMAP_BIT(reset)));
		if (mmio_read32(clockdev->regs + CCU_RESET_BASE +
		                BITMAP_WORDOFF(reset)) &
		    BIT(BITMAP_BIT(reset)))
			return EIO;
	}
	if ((gate = CCU_GET_GATE(dev->clock))) {
		mmio_clearbits32(clockdev->regs + CCU_GATE_BASE +
		                 BITMAP_WORDOFF(gate),
		                 BIT(BITMAP_BIT(gate)));
		if (mmio_read32(clockdev->regs + CCU_GATE_BASE +
		                BITMAP_WORDOFF(gate)) &
		    BIT(BITMAP_BIT(gate)))
			return EIO;
	}

	return SUCCESS;
}

static int
sunxi_ccu_enable(struct device *clockdev, struct device *dev)
{
	uint8_t gate;
	uint8_t reset;

	if ((gate = CCU_GET_GATE(dev->clock))) {
		mmio_setbits32(clockdev->regs + CCU_GATE_BASE +
		               BITMAP_WORDOFF(gate),
		               BIT(BITMAP_BIT(gate)));
		if (!(mmio_read32(clockdev->regs + CCU_GATE_BASE +
		                  BITMAP_WORDOFF(gate)) &
		      BIT(BITMAP_BIT(gate))))
			return EIO;
	}
	if ((reset = CCU_GET_RESET(dev->clock))) {
		mmio_setbits32(clockdev->regs + CCU_RESET_BASE +
		               BITMAP_WORDOFF(reset),
		               BIT(BITMAP_BIT(reset)));
		if (!(mmio_read32(clockdev->regs + CCU_RESET_BASE +
		                  BITMAP_WORDOFF(reset)) &
		      BIT(BITMAP_BIT(reset))))
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
