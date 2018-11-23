/*
 * Copyright © 2016, ARM Limited and Contributors. All rights reserved.
 * Copyright © 2017-2018 The Crust Firmware Authors.
 * SPDX-License-Identifier: BSD-3-Clause OR GPL-2.0-only
 */

#include <debug.h>
#include <dvfs.h>
#include <error.h>
#include <limits.h>
#include <mmio.h>
#include <regulator.h>
#include <stddef.h>
#include <dvfs/cpux.h>

#define OPP_COUNT         ARRAY_SIZE(cpux_opp_table)

#define PLL_CPUX_REG      0x0000

#define PLL_ENABLE_BIT    BIT(31)
#define PLL_STABLE_BIT    BIT(28)

#define PLL_FACTORS(n, k) (((n) - 1) << 8 | ((k) - 1) << 8)

static const struct dvfs_opp cpux_opp_table[] = {
	{ .rate = 408, .voltage = 1000 },
	{ .rate = 648, .voltage = 1040 },
	{ .rate = 816, .voltage = 1080 },
	{ .rate = 912, .voltage = 1120 },
	{ .rate = 960, .voltage = 1160 },
	{ .rate = 1008, .voltage = 1200 },
	{ .rate = 1056, .voltage = 1240 },
	{ .rate = 1104, .voltage = 1260 },
	{ .rate = 1152, .voltage = 1300 },
};

static const uint16_t cpux_pll_factors[OPP_COUNT] = {
	PLL_FACTORS(17, 1), /*  408 MHz */
	PLL_FACTORS(27, 1), /*  648 MHz */
	PLL_FACTORS(17, 2), /*  816 MHz */
	PLL_FACTORS(19, 2), /*  912 MHz */
	PLL_FACTORS(20, 2), /*  960 MHz */
	PLL_FACTORS(21, 2), /* 1008 MHz */
	PLL_FACTORS(22, 2), /* 1056 MHz */
	PLL_FACTORS(23, 2), /* 1104 MHz */
	PLL_FACTORS(24, 2), /* 1152 MHz */
};

static struct dvfs_info cpux_dvfs_info = {
	.latency   = 2500, /* 2.5 ms */
	.opp_count = OPP_COUNT,
	.opp_table = cpux_opp_table,
};

static struct dvfs_info *
cpux_get_info(struct device *dev __unused, uint8_t id __unused)
{
	assert(id < CPUX_DVFS_DOMAIN_COUNT);

	return &cpux_dvfs_info;
}

static uint8_t
cpux_get_opp(struct device *dev, uint8_t id __unused)
{
	return dev->drvdata;
}

static int
cpux_set_pll(struct device *dev, uint8_t opp)
{
	uint32_t reg = cpux_pll_factors[opp] | PLL_ENABLE_BIT;

	/* Change the PLL_CPUX multipliers, and enable the PLL. */
	mmio_write32(dev->regs + PLL_CPUX_REG, reg);

	/* Wait for the PLL to be stable. */
	mmio_poll32(dev->regs + PLL_CPUX_REG, PLL_STABLE_BIT);

	return SUCCESS;
}

static int
cpux_set_opp(struct device *dev, uint8_t id __unused, uint8_t opp)
{
	int      err;
	uint8_t  previous = dev->drvdata;
	uint16_t voltage  = cpux_opp_table[opp].voltage;

	assert(opp < OPP_COUNT);

	/* Lower frequency before voltage. */
	if (opp < previous && (err = cpux_set_pll(dev, opp)))
		return err;
	if ((err = regulator_set_value(dev->supplydev, dev->supply, voltage)))
		return err;
	/* Raise frequency after voltage. */
	if (opp > previous && (err = cpux_set_pll(dev, opp)))
		return err;

	/* Record the new OPP. */
	dev->drvdata = opp;

	return SUCCESS;
}

static int
cpux_probe(struct device *dev)
{
	/* DVFS only works if a voltage regulator is available. */
	if (dev->supplydev == NULL)
		return ENODEV;

	/* Boost CPU speed to minimize boot time. */
	return cpux_set_opp(dev, 0, OPP_COUNT - 1);
}

const struct dvfs_driver cpux_driver = {
	.drv = {
		.class = DM_CLASS_DVFS,
		.probe = cpux_probe,
	},
	.ops = {
		.get_info = cpux_get_info,
		.get_opp  = cpux_get_opp,
		.set_opp  = cpux_set_opp,
	},
};
