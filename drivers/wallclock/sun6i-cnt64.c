/*
 * Copyright © 2017-2018 The Crust Firmware Authors.
 * SPDX-License-Identifier: (BSD-3-Clause OR GPL-2.0)
 */

#include <compiler.h>
#include <error.h>
#include <mmio.h>
#include <util.h>
#include <drivers/clock.h>
#include <drivers/wallclock.h>
#include <drivers/wallclock/sun6i-cnt64.h>

#define OSC24M_CNT64_CTRL_REG 0x280
#define OSC24M_CNT64_LOW_REG  0x284
#define OSC24M_CNT64_HIGH_REG 0x288

static uint64_t
sun6i_cnt64_read(struct device *dev)
{
	uint32_t high_reg;
	uint32_t low_reg;
	uint64_t time;

	mmio_setbits32(dev->regs + OSC24M_CNT64_CTRL_REG, BIT(1));
	while (mmio_read32(dev->regs + OSC24M_CNT64_CTRL_REG) & BIT(1)) {
		/* Wait for counter to latch. */
	}

	low_reg  = mmio_read32(dev->regs + OSC24M_CNT64_LOW_REG);
	high_reg = mmio_read32(dev->regs + OSC24M_CNT64_HIGH_REG);
	time     = ((uint64_t)high_reg << 32) | low_reg;

	return time;
}

static const struct wallclock_driver_ops sun6i_cnt64_driver_ops = {
	.read = sun6i_cnt64_read,
};

static int
sun6i_cnt64_probe(struct device *dev __unused)
{
	return SUCCESS;
}

const struct driver sun6i_cnt64_driver = {
	.name  = "sun6i-cnt64",
	.class = DM_CLASS_WALLCLOCK,
	.probe = sun6i_cnt64_probe,
	.ops   = &sun6i_cnt64_driver_ops,
};
