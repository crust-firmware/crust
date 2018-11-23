/*
 * Copyright © 2017-2018 The Crust Firmware Authors.
 * SPDX-License-Identifier: BSD-3-Clause OR GPL-2.0-only
 */

#include <debug.h>
#include <error.h>
#include <i2c.h>
#include <limits.h>
#include <mmio.h>
#include <regulator.h>
#include <mfd/axp803.h>
#include <regulator/axp803.h>

#define OUTPUT_POWER_CONTROL1 0x10
#define OUTPUT_POWER_CONTROL2 0x12
#define OUTPUT_POWER_CONTROL3 0x13

struct axp803_regulator_info {
	struct regulator_info info;
	uint8_t               value_register;
	uint8_t               enable_register;
	uint8_t               enable_mask;
	uint8_t               status_mask;
};

static struct axp803_regulator_info axp803_regulators[AXP803_REGL_COUNT] = {
	[AXP803_REGL_DCDC1] = {
		.info = {
			.name      = "dcdc1",
			.min_value = 1600,
			.max_value = 3400,
			.ranges    = {
				{
					.start_value = 1600,
					.step        = 100,
				},
			},
			.flags = REGL_READABLE | REGL_WRITABLE | REGL_CRITICAL,
		},
		.value_register  = 0x20,
		.enable_register = OUTPUT_POWER_CONTROL1,
		.enable_mask     = BIT(0),
		.status_mask     = BIT(7),
	},
	[AXP803_REGL_DCDC2] = {
		.info = {
			.name      = "dcdc2",
			.min_value = 500,
			.max_value = 1300,
			.ranges    = {
				{
					.start_value = 500,
					.step        = 10,
				},
				{
					.start_raw   = 0x47,
					.start_value = 1220,
					.step        = 20,
				},
			},
			.flags = REGL_READABLE,
		},
		.value_register  = 0x21,
		.enable_register = OUTPUT_POWER_CONTROL1,
		.enable_mask     = BIT(1),
		.status_mask     = BIT(7),
	},
	[AXP803_REGL_DCDC3] = {
		.info = {
			.name      = "dcdc3",
			.min_value = 500,
			.max_value = 1300,
			.ranges    = {
				{
					.start_value = 500,
					.step        = 10,
				},
				{
					.start_raw   = 0x47,
					.start_value = 1220,
					.step        = 20,
				},
			},
			.flags = REGL_READABLE,
		},
		.value_register  = 0x22,
		.enable_register = OUTPUT_POWER_CONTROL1,
		.enable_mask     = BIT(2),
		.status_mask     = BIT(7),
	},
	[AXP803_REGL_DCDC4] = {
		.info = {
			.name      = "dcdc4",
			.min_value = 500,
			.max_value = 1300,
			.ranges    = {
				{
					.start_value = 500,
					.step        = 10,
				},
				{
					.start_raw   = 0x47,
					.start_value = 1220,
					.step        = 20,
				},
			},
			.flags = REGL_READABLE | REGL_WRITABLE,
		},
		.value_register  = 0x23,
		.enable_register = OUTPUT_POWER_CONTROL1,
		.enable_mask     = BIT(3),
		.status_mask     = BIT(7),
	},
	[AXP803_REGL_DCDC5] = {
		.info = {
			.name      = "dcdc5",
			.min_value = 800,
			.max_value = 1840,
			.ranges    = {
				{
					.start_value = 800,
					.step        = 10,
				},
				{
					.start_raw   = 0x21,
					.start_value = 1140,
					.step        = 20,
				},
			},
			.flags = REGL_READABLE,
		},
		.value_register  = 0x24,
		.enable_register = OUTPUT_POWER_CONTROL1,
		.enable_mask     = BIT(4),
		.status_mask     = BIT(7),
	},
	[AXP803_REGL_DCDC6] = {
		.info = {
			.name      = "dcdc6",
			.min_value = 600,
			.max_value = 1520,
			.ranges    = {
				{
					.start_value = 600,
					.step        = 10,
				},
				{
					.start_raw   = 0x33,
					.start_value = 1120,
					.step        = 20,
				},
			},
			.flags = REGL_READABLE | REGL_WRITABLE | REGL_CRITICAL,
		},
		.value_register  = 0x25,
		.enable_register = OUTPUT_POWER_CONTROL1,
		.enable_mask     = BIT(5),
		.status_mask     = BIT(7),
	},
	[AXP803_REGL_DC1SW] = {
		.info = {
			.name      = "dc1sw",
			.min_value = 1600,
			.max_value = 3400,
			.ranges    = {
				{
					.start_value = 1600,
					.step        = 100,
				},
			},
			.flags = REGL_READABLE | REGL_WRITABLE,
		},
		.value_register  = 0x20,
		.enable_register = OUTPUT_POWER_CONTROL2,
		.enable_mask     = BIT(7),
	},
	[AXP803_REGL_ALDO1] = {
		.info = {
			.name      = "aldo1",
			.min_value = 700,
			.max_value = 3300,
			.ranges    = {
				{
					.start_value = 700,
					.step        = 100,
				},
			},
			.flags = REGL_READABLE | REGL_WRITABLE,
		},
		.value_register  = 0x28,
		.enable_register = OUTPUT_POWER_CONTROL3,
		.enable_mask     = BIT(5),
	},
	[AXP803_REGL_ALDO2] = {
		.info = {
			.name      = "aldo2",
			.min_value = 700,
			.max_value = 3300,
			.ranges    = {
				{
					.start_value = 700,
					.step        = 100,
				},
			},
			.flags = REGL_READABLE | REGL_CRITICAL,
		},
		.value_register  = 0x29,
		.enable_register = OUTPUT_POWER_CONTROL3,
		.enable_mask     = BIT(6),
	},
	[AXP803_REGL_ALDO3] = {
		.info = {
			.name      = "aldo3",
			.min_value = 700,
			.max_value = 3300,
			.ranges    = {
				{
					.start_value = 700,
					.step        = 100,
				},
			},
			.flags = REGL_READABLE | REGL_WRITABLE | REGL_CRITICAL,
		},
		.value_register  = 0x2a,
		.enable_register = OUTPUT_POWER_CONTROL3,
		.enable_mask     = BIT(7),
	},
	[AXP803_REGL_DLDO1] = {
		.info = {
			.name      = "dldo1",
			.min_value = 700,
			.max_value = 3300,
			.ranges    = {
				{
					.start_value = 700,
					.step        = 100,
				},
			},
			.flags = REGL_READABLE | REGL_WRITABLE,
		},
		.value_register  = 0x15,
		.enable_register = OUTPUT_POWER_CONTROL2,
		.enable_mask     = BIT(3),
	},
	[AXP803_REGL_DLDO2] = {
		.info = {
			.name      = "dldo2",
			.min_value = 700,
			.max_value = 4200,
			.ranges    = {
				{
					.start_value = 700,
					.step        = 100,
				},
				{
					.start_raw   = 0x1b,
					.start_value = 3400,
					.step        = 200,
				},
			},
			.flags = REGL_READABLE | REGL_WRITABLE,
		},
		.value_register  = 0x16,
		.enable_register = OUTPUT_POWER_CONTROL2,
		.enable_mask     = BIT(4),
	},
	[AXP803_REGL_DLDO3] = {
		.info = {
			.name      = "dldo3",
			.min_value = 700,
			.max_value = 3300,
			.ranges    = {
				{
					.start_value = 700,
					.step        = 100,
				},
			},
			.flags = REGL_READABLE | REGL_WRITABLE,
		},
		.value_register  = 0x17,
		.enable_register = OUTPUT_POWER_CONTROL2,
		.enable_mask     = BIT(5),
	},
	[AXP803_REGL_DLDO4] = {
		.info = {
			.name      = "dldo4",
			.min_value = 700,
			.max_value = 3300,
			.ranges    = {
				{
					.start_value = 700,
					.step        = 100,
				},
			},
			.flags = REGL_READABLE | REGL_WRITABLE,
		},
		.value_register  = 0x18,
		.enable_register = OUTPUT_POWER_CONTROL2,
		.enable_mask     = BIT(6),
	},
	[AXP803_REGL_ELDO1] = {
		.info = {
			.name      = "eldo1",
			.min_value = 700,
			.max_value = 1900,
			.ranges    = {
				{
					.start_value = 700,
					.step        = 50,
				},
			},
			.flags = REGL_READABLE | REGL_WRITABLE,
		},
		.value_register  = 0x19,
		.enable_register = OUTPUT_POWER_CONTROL2,
		.enable_mask     = BIT(0),
	},
	[AXP803_REGL_ELDO2] = {
		.info = {
			.name      = "eldo2",
			.min_value = 700,
			.max_value = 1900,
			.ranges    = {
				{
					.start_value = 700,
					.step        = 50,
				},
			},
			.flags = REGL_READABLE | REGL_WRITABLE,
		},
		.value_register  = 0x1a,
		.enable_register = OUTPUT_POWER_CONTROL2,
		.enable_mask     = BIT(1),
	},
	[AXP803_REGL_ELDO3] = {
		.info = {
			.name      = "eldo3",
			.min_value = 700,
			.max_value = 1900,
			.ranges    = {
				{
					.start_value = 700,
					.step        = 50,
				},
			},
			.flags = REGL_READABLE | REGL_WRITABLE,
		},
		.value_register  = 0x1b,
		.enable_register = OUTPUT_POWER_CONTROL2,
		.enable_mask     = BIT(2),
	},
	[AXP803_REGL_FLDO1] = {
		.info = {
			.name      = "fldo1",
			.min_value = 700,
			.max_value = 1450,
			.ranges    = {
				{
					.start_value = 700,
					.step        = 50,
				},
			},
			.flags = REGL_READABLE | REGL_WRITABLE,
		},
		.value_register  = 0x1c,
		.enable_register = OUTPUT_POWER_CONTROL3,
		.enable_mask     = BIT(2),
	},
	[AXP803_REGL_FLDO2] = {
		.info = {
			.name      = "fldo2",
			.min_value = 700,
			.max_value = 1450,
			.ranges    = {
				{
					.start_value = 700,
					.step        = 50,
				},
			},
			.flags = REGL_READABLE | REGL_CRITICAL,
		},
		.value_register  = 0x1d,
		.enable_register = OUTPUT_POWER_CONTROL3,
		.enable_mask     = BIT(3),
	},
	[AXP803_REGL_GPIO0] = {
		.info = {
			.name      = "gpio0",
			.min_value = 700,
			.max_value = 3300,
			.ranges    = {
				{
					.start_value = 700,
					.step        = 100,
				},
			},
			.flags = REGL_READABLE | REGL_WRITABLE,
		},
		.value_register  = 0x91,
		.enable_register = 0x90,
		.enable_mask     = BIT(2),
	},
	[AXP803_REGL_GPIO1] = {
		.info = {
			.name      = "gpio1",
			.min_value = 700,
			.max_value = 3300,
			.ranges    = {
				{
					.start_value = 700,
					.step        = 100,
				},
			},
			.flags = REGL_READABLE | REGL_WRITABLE,
		},
		.value_register  = 0x93,
		.enable_register = 0x92,
		.enable_mask     = BIT(2),
	},
};

static struct regulator_info *
axp803_regulator_get_info(struct device *dev __unused, uint8_t id)
{
	assert(id < AXP803_REGL_COUNT);

	return &axp803_regulators[id].info;
}

static int
axp803_regulator_get_state(struct device *dev, uint8_t id)
{
	int     err;
	uint8_t reg;
	uint8_t regaddr = axp803_regulators[id].enable_register;
	uint8_t regmask = axp803_regulators[id].enable_mask;

	if ((err = i2c_read_reg(dev->bus, dev->addr, regaddr, &reg)))
		return err;

	/* GPIO LDOs have their status bit inverted. */
	return !!(reg & regmask) ^ (id >= AXP803_REGL_GPIO0);
}

static int
axp803_regulator_read_raw(struct device *dev, uint8_t id, uint32_t *raw)
{
	int     err;
	uint8_t reg;
	uint8_t regaddr = axp803_regulators[id].value_register;
	uint8_t regmask = axp803_regulators[id].status_mask;

	if ((err = i2c_read_reg(dev->bus, dev->addr, regaddr, &reg)))
		return err;
	/* Mask out a possible status bit. */
	*raw = reg & ~regmask;

	return SUCCESS;
}

static int
axp803_regulator_set_state(struct device *dev, uint8_t id, bool enabled)
{
	int     err;
	uint8_t reg;
	uint8_t regaddr = axp803_regulators[id].enable_register;
	uint8_t regmask = axp803_regulators[id].enable_mask;

	if ((err = i2c_read_reg(dev->bus, dev->addr, regaddr, &reg)))
		return err;
	/* GPIO LDOs have their status bit inverted. */
	enabled ^= (id >= AXP803_REGL_GPIO0);
	reg      = enabled ? reg | regmask : reg & ~regmask;

	return i2c_write_reg(dev->bus, dev->addr, regaddr, reg);
}

static int
axp803_regulator_write_raw(struct device *dev, uint8_t id, uint32_t raw)
{
	uint8_t regaddr = axp803_regulators[id].value_register;

	assert(raw <= UINT8_MAX);

	/* AXP803_REGL_DC1SW is a secondary output of AXP803_REGL_DCDC1,
	 * without its own voltage control. Only pretend to set its voltage. */
	if (id == AXP803_REGL_DC1SW)
		return SUCCESS;

	return i2c_write_reg(dev->bus, dev->addr, regaddr, raw);
}

static int
axp_regulator_probe(struct device *dev)
{
	int err;

	if ((err = i2c_probe(dev->bus, dev->addr)))
		return err;
	if ((err = axp803_match_type(dev)))
		return err;
	dev->subdev_count = AXP803_REGL_COUNT;
	if ((err = regulator_set_defaults(dev, (uint16_t *)dev->drvdata)))
		return err;

	return SUCCESS;
}

const struct regulator_driver axp803_regulator_driver = {
	.drv = {
		.class = DM_CLASS_REGULATOR,
		.probe = axp_regulator_probe,
	},
	.ops = {
		.get_info  = axp803_regulator_get_info,
		.get_state = axp803_regulator_get_state,
		.read_raw  = axp803_regulator_read_raw,
		.set_state = axp803_regulator_set_state,
		.write_raw = axp803_regulator_write_raw,
	},
};
