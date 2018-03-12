/*
 * Copyright © 2017-2018 The Crust Firmware Authors.
 * SPDX-License-Identifier: (BSD-3-Clause OR GPL-2.0)
 */

#include <debug.h>
#include <error.h>
#include <i2c.h>
#include <limits.h>
#include <mmio.h>
#include <regulator.h>
#include <regulator/axp803.h>

#define OUTPUT_POWER_CONTROL_1 0x10
#define OUTPUT_POWER_CONTROL_2 0x12
#define OUTPUT_POWER_CONTROL_3 0x13

enum {
	AXP803_REGL_DCDC1,
	AXP803_REGL_DCDC2,
	AXP803_REGL_DCDC3,
	AXP803_REGL_DCDC4,
	AXP803_REGL_DCDC5,
	AXP803_REGL_DCDC6,
	AXP803_REGL_ALDO1,
	AXP803_REGL_ALDO2,
	AXP803_REGL_ALDO3,
	AXP803_REGL_DLDO1,
	AXP803_REGL_DLDO2,
	AXP803_REGL_DLDO3,
	AXP803_REGL_DLDO4,
	AXP803_REGL_ELDO1,
	AXP803_REGL_ELDO2,
	AXP803_REGL_ELDO3,
	AXP803_REGL_FLDO1,
	AXP803_REGL_FLDO2,
	AXP803_REGL_GPIO0LDO,
	AXP803_REGL_GPIO1LDO,
	AXP803_REGL_COUNT,
};

static struct axp803_regulator_info axp803_regl_info[AXP803_REGL_COUNT] = {
	[AXP803_REGL_DCDC1] = {
		.info = {
			.name      = "DCDC1",
			.min_value = 1600,
			.max_value = 3400,
			.ranges    = {
				{
					.start_raw   = 0x11,
					.start_value = 1600,
					.step        = 100,
				},
			},
			.flags = REGL_READABLE | REGL_WRITABLE,
		},
		.value_register  = 0x20,
		.enable_register = OUTPUT_POWER_CONTROL_1,
		.bit = 0,
	},
	[AXP803_REGL_DCDC2] = {
		.info = {
			.name      = "DCDC2",
			.min_value = 500,
			.max_value = 1300,
			.ranges    = {
				{
					.start_raw   = 0xA8,
					.start_value = 500,
					.step        = 10,
				},
				{
					.start_raw   = 0xA8,
					.start_value = 1220,
					.step        = 20,
				},
			},
			.flags = REGL_READABLE | REGL_WRITABLE,
		},
		.value_register  = 0x21,
		.enable_register = OUTPUT_POWER_CONTROL_1,
		.bit = 1,
	},
	[AXP803_REGL_DCDC3] = {
		.info = {
			.name      = "DCDC3",
			.min_value = 500,
			.max_value = 1300,
			.ranges    = {
				{
					.start_raw   = 0xA8,
					.start_value = 500,
					.step        = 10,
				},
				{
					.start_raw   = 0xA8,
					.start_value = 1220,
					.step        = 20,
				},
			},
			.flags = REGL_READABLE | REGL_WRITABLE,
		},
		.value_register  = 0x22,
		.enable_register = OUTPUT_POWER_CONTROL_1,
		.bit = 2,
	},
	[AXP803_REGL_DCDC4] = {
		.info = {
			.name      = "DCDC4",
			.min_value = 500,
			.max_value = 1300,
			.ranges    = {
				{
					.start_raw   = 0xA8,
					.start_value = 500,
					.step        = 10,
				},
				{
					.start_raw   = 0xA8,
					.start_value = 1220,
					.step        = 20,
				},
			},
			.flags = REGL_READABLE | REGL_WRITABLE,
		},
		.value_register  = 0x23,
		.enable_register = OUTPUT_POWER_CONTROL_1,
		.bit = 3,
	},
	[AXP803_REGL_DCDC5] = {
		.info = {
			.name      = "DCDC5",
			.min_value = 600,
			.max_value = 1840,
			.ranges    = {
				{
					.start_raw   = 0xB3,
					.start_value = 600,
					.step        = 10,
				},
				{
					.start_raw   = 0xB3,
					.start_value = 1140,
					.step        = 20,
				},
			},
			.flags = REGL_READABLE | REGL_WRITABLE,
		},
		.value_register  = 0x24,
		.enable_register = OUTPUT_POWER_CONTROL_1,
		.bit = 4,
	},
	[AXP803_REGL_DCDC6] = {
		.info = {
			.name      = "DCDC6",
			.min_value = 600,
			.max_value = 1520,
			.ranges    = {
				{
					.start_raw   = 0x9E,
					.start_value = 600,
					.step        = 10,
				},
				{
					.start_raw   = 0x9E,
					.start_value = 1120,
					.step        = 20,
				},
			},
			.flags = REGL_READABLE | REGL_WRITABLE,
		},
		.value_register  = 0x25,
		.enable_register = OUTPUT_POWER_CONTROL_1,
		.bit = 5,
	},
	[AXP803_REGL_ALDO1] = {
		.info = {
			.name      = "ALDO1",
			.min_value = 700,
			.max_value = 3300,
			.ranges    = {
				{
					.start_raw   = 0x17,
					.start_value = 700,
					.step        = 100,
				},
			},
			.flags = REGL_READABLE | REGL_WRITABLE,
		},
		.value_register  = 0x28,
		.enable_register = OUTPUT_POWER_CONTROL_3,
		.bit = 5,
	},
	[AXP803_REGL_ALDO2] = {
		.info = {
			.name      = "ALDO2",
			.min_value = 700,
			.max_value = 3300,
			.ranges    = {
				{
					.start_raw   = 0x17,
					.start_value = 700,
					.step        = 100,
				},
			},
			.flags = REGL_READABLE | REGL_WRITABLE,
		},
		.value_register  = 0x29,
		.enable_register = OUTPUT_POWER_CONTROL_3,
		.bit = 6,
	},
	[AXP803_REGL_ALDO3] = {
		.info = {
			.name      = "ALDO3",
			.min_value = 700,
			.max_value = 3300,
			.ranges    = {
				{
					.start_raw   = 0x17,
					.start_value = 700,
					.step        = 100,
				},
			},
			.flags = REGL_READABLE | REGL_WRITABLE,
		},
		.value_register  = 0x2A,
		.enable_register = OUTPUT_POWER_CONTROL_3,
		.bit = 7,
	},
	[AXP803_REGL_DLDO1] = {
		.info = {
			.name      = "DLDO1",
			.min_value = 700,
			.max_value = 3300,
			.ranges    = {
				{
					.start_raw   = 0x16,
					.start_value = 700,
					.step        = 100,
				},
			},
			.flags = REGL_READABLE | REGL_WRITABLE,
		},
		.value_register  = 0x15,
		.enable_register = OUTPUT_POWER_CONTROL_2,
		.bit = 3,
	},
	[AXP803_REGL_DLDO2] = {
		.info = {
			.name      = "DLDO2",
			.min_value = 700,
			.max_value = 4200,
			.ranges    = {
				{
					.start_raw   = 0x16,
					.start_value = 700,
					.step        = 100,
				},
				{
					.start_raw   = 0x16,
					.start_value = 3400,
					.step        = 200,
				},
			},
			.flags = REGL_READABLE | REGL_WRITABLE,
		},
		.value_register  = 0x16,
		.enable_register = OUTPUT_POWER_CONTROL_2,
		.bit = 4,
	},
	[AXP803_REGL_DLDO3] = {
		.info = {
			.name      = "DLDO3",
			.min_value = 700,
			.max_value = 3300,
			.ranges    = {
				{
					.start_raw   = 0x16,
					.start_value = 700,
					.step        = 100,
				},
			},
			.flags = REGL_READABLE | REGL_WRITABLE,
		},
		.value_register  = 0x17,
		.enable_register = OUTPUT_POWER_CONTROL_2,
		.bit = 5,
	},
	[AXP803_REGL_DLDO4] = {
		.info = {
			.name      = "DLDO4",
			.min_value = 700,
			.max_value = 1900,
			.ranges    = {
				{
					.start_raw   = 0x1A,
					.start_value = 700,
					.step        = 100,
				},
			},
			.flags = REGL_READABLE | REGL_WRITABLE,
		},
		.value_register  = 0x15,
		.enable_register = OUTPUT_POWER_CONTROL_2,
		.bit = 6,
	},
	[AXP803_REGL_ELDO1] = {
		.info = {
			.name      = "ELDO1",
			.min_value = 700,
			.max_value = 1900,
			.ranges    = {
				{
					.start_raw   = 0x00,
					.start_value = 700,
					.step        = 50,
				},
			},
			.flags = REGL_READABLE | REGL_WRITABLE,
		},
		.value_register  = 0x19,
		.enable_register = OUTPUT_POWER_CONTROL_2,
		.bit = 0,
	},
	[AXP803_REGL_ELDO2] = {
		.info = {
			.name      = "ELDO2",
			.min_value = 700,
			.max_value = 1900,
			.ranges    = {
				{
					.start_raw   = 0x00,
					.start_value = 700,
					.step        = 50,
				},
			},
			.flags = REGL_READABLE | REGL_WRITABLE,
		},
		.value_register  = 0x1A,
		.enable_register = OUTPUT_POWER_CONTROL_2,
		.bit = 1,
	},
	[AXP803_REGL_ELDO3] = {
		.info = {
			.name      = "ELDO3",
			.min_value = 700,
			.max_value = 1900,
			.ranges    = {
				{
					.start_raw   = 0x00,
					.start_value = 700,
					.step        = 50,
				},
			},
			.flags = REGL_READABLE | REGL_WRITABLE,
		},
		.value_register  = 0x1B,
		.enable_register = OUTPUT_POWER_CONTROL_2,
		.bit = 2,
	},
	[AXP803_REGL_FLDO1] = {
		.info = {
			.name      = "FLDO1",
			.min_value = 700,
			.max_value = 1450,
			.ranges    = {
				{
					.start_raw   = 0x0B,
					.start_value = 700,
					.step        = 50,
				},
			},
			.flags = REGL_READABLE | REGL_WRITABLE,
		},
		.value_register  = 0x1C,
		.enable_register = OUTPUT_POWER_CONTROL_3,
		.bit = 2,
	},
	[AXP803_REGL_FLDO2] = {
		.info = {
			.name      = "FLDO2",
			.min_value = 700,
			.max_value = 1450,
			.ranges    = {
				{
					.start_raw   = 0x04,
					.start_value = 700,
					.step        = 50,
				},
			},
			.flags = REGL_READABLE | REGL_WRITABLE,
		},
		.value_register  = 0x1D,
		.enable_register = OUTPUT_POWER_CONTROL_3,
		.bit = OUTPUT_POWER_CONTROL_3,
	},
	[AXP803_REGL_GPIO0LDO] = {
		.info = {
			.name      = "GPIO0LDO",
			.min_value = 700,
			.max_value = 3300,
			.ranges    = {
				{
					.start_raw   = 0x1A,
					.start_value = 700,
					.step        = 100,
				},
			},
			.flags = REGL_READABLE | REGL_WRITABLE,
		},
		.value_register  = 0x91,
		.enable_register = 0x00,
		.bit = 0x00,
	},
	[AXP803_REGL_GPIO1LDO] = {
		.info = {
			.name      = "GPIO1LDO",
			.min_value = 700,
			.max_value = 3300,
			.ranges    = {
				{
					.start_raw   = 0x1A,
					.start_value = 700,
					.step        = 100,
				},
			},
			.flags = REGL_READABLE | REGL_WRITABLE,
		},
		.value_register  = 0x93,
		.enable_register = 0x00,
		.bit = 0x00,
	},
};

static uint8_t
axp803_get_count(struct device *dev __unused)
{
	return AXP803_REGL_COUNT;
}

static struct regulator_info *
axp803_get_info(struct device *dev __unused, uint8_t id)
{
	assert(id < REGULATOR_COUNT);

	return &axp803_regl_info[id].info;
}

static int
axp803_get_state(struct device *dev, uint8_t id)
{
	int     err;
	uint8_t reg;

	if ((err =
		     i2c_read_reg(dev->bus, dev->addr,
		                  axp803_regl_info[id].enable_register,
		                  &reg)))
		return err;

	return (reg & BIT(0)) == 0;
}

static int
axp803_read_raw(struct device *dev, uint8_t id, uint32_t *raw)
{
	int     err;
	uint8_t reg;

	if ((err =
		     i2c_read_reg(dev->bus, dev->addr,
		                  axp803_regl_info[id].value_register,
		                  &reg)))
		return err;
	*raw = reg & ~BIT(7);

	return SUCCESS;
}

static int
axp803_set_state(struct device *dev, uint8_t id, bool enabled)
{
	int     err;
	uint8_t reg;

	if ((err =
		     i2c_read_reg(dev->bus, dev->addr,
		                  axp803_regl_info[id].enable_register,
		                  &reg)))
		return err;
	reg = enabled ? reg & ~BIT(0) : reg | BIT(0);

	return i2c_write_reg(dev->bus, dev->addr,
	                     axp803_regl_info[id].enable_register, reg);
}

static int
axp803_write_raw(struct device *dev, uint8_t id, uint32_t raw)
{
	assert(raw <= UINT8_MAX);

	return i2c_write_reg(dev->bus, dev->addr,
	                     axp803_regl_info[id].value_register,
	                     raw | BIT(7));
}

static int
axp803_probe(struct device *dev)
{
	int err;

	if ((err = i2c_probe(dev->bus, dev->addr)))
		return err;

	return SUCCESS;
}

const struct regulator_driver axp803_driver = {
	.drv = {
		.name  = "axp803",
		.class = DM_CLASS_REGULATOR,
		.probe = axp803_probe,
	},
	.ops = {
		.get_count = axp803_get_count,
		.get_info  = axp803_get_info,
		.get_state = axp803_get_state,
		.read_raw  = axp803_read_raw,
		.set_state = axp803_set_state,
		.write_raw = axp803_write_raw,
	},
};
