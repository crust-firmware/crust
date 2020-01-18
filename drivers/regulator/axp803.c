/*
 * Copyright © 2017-2020 The Crust Firmware Authors.
 * SPDX-License-Identifier: BSD-3-Clause OR GPL-2.0-only
 */

#include <debug.h>
#include <error.h>
#include <limits.h>
#include <mmio.h>
#include <regulator.h>
#include <mfd/axp803.h>
#include <regulator/axp803.h>
#include <rsb/sunxi-rsb.h>

#include "regulator.h"

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
			.min_value = 1600,
			.max_value = 3400,
			.ranges    = {
				{
					.start_value = 1600,
					.step        = 100,
				},
			},
		},
		.value_register  = 0x20,
		.enable_register = OUTPUT_POWER_CONTROL1,
		.enable_mask     = BIT(0),
		.status_mask     = BIT(7),
	},
	[AXP803_REGL_DCDC2] = {
		.info = {
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
		},
		.value_register  = 0x21,
		.enable_register = OUTPUT_POWER_CONTROL1,
		.enable_mask     = BIT(1),
		.status_mask     = BIT(7),
	},
	[AXP803_REGL_DCDC3] = {
		.info = {
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
		},
		.value_register  = 0x22,
		.enable_register = OUTPUT_POWER_CONTROL1,
		.enable_mask     = BIT(2),
		.status_mask     = BIT(7),
	},
	[AXP803_REGL_DCDC4] = {
		.info = {
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
		},
		.value_register  = 0x23,
		.enable_register = OUTPUT_POWER_CONTROL1,
		.enable_mask     = BIT(3),
		.status_mask     = BIT(7),
	},
	[AXP803_REGL_DCDC5] = {
		.info = {
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
		},
		.value_register  = 0x24,
		.enable_register = OUTPUT_POWER_CONTROL1,
		.enable_mask     = BIT(4),
		.status_mask     = BIT(7),
	},
	[AXP803_REGL_DCDC6] = {
		.info = {
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
		},
		.value_register  = 0x25,
		.enable_register = OUTPUT_POWER_CONTROL1,
		.enable_mask     = BIT(5),
		.status_mask     = BIT(7),
	},
	[AXP803_REGL_DC1SW] = {
		.info = {
			.min_value = 1600,
			.max_value = 3400,
			.ranges    = {
				{
					.start_value = 1600,
					.step        = 100,
				},
			},
		},
		.value_register  = 0x20,
		.enable_register = OUTPUT_POWER_CONTROL2,
		.enable_mask     = BIT(7),
	},
	[AXP803_REGL_ALDO1] = {
		.info = {
			.min_value = 700,
			.max_value = 3300,
			.ranges    = {
				{
					.start_value = 700,
					.step        = 100,
				},
			},
		},
		.value_register  = 0x28,
		.enable_register = OUTPUT_POWER_CONTROL3,
		.enable_mask     = BIT(5),
	},
	[AXP803_REGL_ALDO2] = {
		.info = {
			.min_value = 700,
			.max_value = 3300,
			.ranges    = {
				{
					.start_value = 700,
					.step        = 100,
				},
			},
		},
		.value_register  = 0x29,
		.enable_register = OUTPUT_POWER_CONTROL3,
		.enable_mask     = BIT(6),
	},
	[AXP803_REGL_ALDO3] = {
		.info = {
			.min_value = 700,
			.max_value = 3300,
			.ranges    = {
				{
					.start_value = 700,
					.step        = 100,
				},
			},
		},
		.value_register  = 0x2a,
		.enable_register = OUTPUT_POWER_CONTROL3,
		.enable_mask     = BIT(7),
	},
	[AXP803_REGL_DLDO1] = {
		.info = {
			.min_value = 700,
			.max_value = 3300,
			.ranges    = {
				{
					.start_value = 700,
					.step        = 100,
				},
			},
		},
		.value_register  = 0x15,
		.enable_register = OUTPUT_POWER_CONTROL2,
		.enable_mask     = BIT(3),
	},
	[AXP803_REGL_DLDO2] = {
		.info = {
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
		},
		.value_register  = 0x16,
		.enable_register = OUTPUT_POWER_CONTROL2,
		.enable_mask     = BIT(4),
	},
	[AXP803_REGL_DLDO3] = {
		.info = {
			.min_value = 700,
			.max_value = 3300,
			.ranges    = {
				{
					.start_value = 700,
					.step        = 100,
				},
			},
		},
		.value_register  = 0x17,
		.enable_register = OUTPUT_POWER_CONTROL2,
		.enable_mask     = BIT(5),
	},
	[AXP803_REGL_DLDO4] = {
		.info = {
			.min_value = 700,
			.max_value = 3300,
			.ranges    = {
				{
					.start_value = 700,
					.step        = 100,
				},
			},
		},
		.value_register  = 0x18,
		.enable_register = OUTPUT_POWER_CONTROL2,
		.enable_mask     = BIT(6),
	},
	[AXP803_REGL_ELDO1] = {
		.info = {
			.min_value = 700,
			.max_value = 1900,
			.ranges    = {
				{
					.start_value = 700,
					.step        = 50,
				},
			},
		},
		.value_register  = 0x19,
		.enable_register = OUTPUT_POWER_CONTROL2,
		.enable_mask     = BIT(0),
	},
	[AXP803_REGL_ELDO2] = {
		.info = {
			.min_value = 700,
			.max_value = 1900,
			.ranges    = {
				{
					.start_value = 700,
					.step        = 50,
				},
			},
		},
		.value_register  = 0x1a,
		.enable_register = OUTPUT_POWER_CONTROL2,
		.enable_mask     = BIT(1),
	},
	[AXP803_REGL_ELDO3] = {
		.info = {
			.min_value = 700,
			.max_value = 1900,
			.ranges    = {
				{
					.start_value = 700,
					.step        = 50,
				},
			},
		},
		.value_register  = 0x1b,
		.enable_register = OUTPUT_POWER_CONTROL2,
		.enable_mask     = BIT(2),
	},
	[AXP803_REGL_FLDO1] = {
		.info = {
			.min_value = 700,
			.max_value = 1450,
			.ranges    = {
				{
					.start_value = 700,
					.step        = 50,
				},
			},
		},
		.value_register  = 0x1c,
		.enable_register = OUTPUT_POWER_CONTROL3,
		.enable_mask     = BIT(2),
	},
	[AXP803_REGL_FLDO2] = {
		.info = {
			.min_value = 700,
			.max_value = 1450,
			.ranges    = {
				{
					.start_value = 700,
					.step        = 50,
				},
			},
		},
		.value_register  = 0x1d,
		.enable_register = OUTPUT_POWER_CONTROL3,
		.enable_mask     = BIT(3),
	},
	[AXP803_REGL_GPIO0] = {
		.info = {
			.min_value = 700,
			.max_value = 3300,
			.ranges    = {
				{
					.start_value = 700,
					.step        = 100,
				},
			},
		},
		.value_register  = 0x91,
		.enable_register = 0x90,
		.enable_mask     = BIT(2),
	},
	[AXP803_REGL_GPIO1] = {
		.info = {
			.min_value = 700,
			.max_value = 3300,
			.ranges    = {
				{
					.start_value = 700,
					.step        = 100,
				},
			},
		},
		.value_register  = 0x93,
		.enable_register = 0x92,
		.enable_mask     = BIT(2),
	},
};

static inline const struct axp803_regulator *
to_axp803_regulator(const struct device *dev)
{
	return container_of(dev, const struct axp803_regulator, dev);
}

static struct regulator_info *
axp803_regulator_get_info(const struct device *dev UNUSED, uint8_t id)
{
	assert(id < AXP803_REGL_COUNT);

	return &axp803_regulators[id].info;
}

static int
axp803_regulator_get_state(const struct device *dev, uint8_t id)
{
	const struct axp803_regulator *self = to_axp803_regulator(dev);
	uint8_t regaddr = axp803_regulators[id].enable_register;
	uint8_t regmask = axp803_regulators[id].enable_mask;
	uint8_t reg;
	int err;

	if ((err = rsb_read(&self->bus, regaddr, &reg)))
		return err;

	/* GPIO LDOs have their status bit inverted. */
	return !!(reg & regmask) ^ (id >= AXP803_REGL_GPIO0);
}

static int
axp803_regulator_read_raw(const struct device *dev, uint8_t id, uint32_t *raw)
{
	const struct axp803_regulator *self = to_axp803_regulator(dev);
	uint8_t regaddr = axp803_regulators[id].value_register;
	uint8_t regmask = axp803_regulators[id].status_mask;
	uint8_t reg;
	int err;

	if ((err = rsb_read(&self->bus, regaddr, &reg)))
		return err;
	/* Mask out a possible status bit. */
	*raw = reg & ~regmask;

	return SUCCESS;
}

static int
axp803_regulator_set_state(const struct device *dev, uint8_t id, bool enabled)
{
	const struct axp803_regulator *self = to_axp803_regulator(dev);
	uint8_t regaddr = axp803_regulators[id].enable_register;
	uint8_t regmask = axp803_regulators[id].enable_mask;
	uint8_t reg;
	int err;

	if ((err = rsb_read(&self->bus, regaddr, &reg)))
		return err;
	/* GPIO LDOs have their status bit inverted. */
	enabled ^= (id >= AXP803_REGL_GPIO0);
	reg      = enabled ? reg | regmask : reg & ~regmask;

	return rsb_write(&self->bus, regaddr, reg);
}

static int
axp803_regulator_write_raw(const struct device *dev, uint8_t id, uint32_t raw)
{
	const struct axp803_regulator *self = to_axp803_regulator(dev);
	uint8_t regaddr = axp803_regulators[id].value_register;

	assert(raw <= UINT8_MAX);

	/* AXP803_REGL_DC1SW is a secondary output of AXP803_REGL_DCDC1,
	 * without its own voltage control. Only pretend to set its voltage. */
	if (id == AXP803_REGL_DC1SW)
		return SUCCESS;

	return rsb_write(&self->bus, regaddr, raw);
}

static int
axp803_regulator_probe(const struct device *dev)
{
	const struct axp803_regulator *self = to_axp803_regulator(dev);
	int err;

	if ((err = axp803_get(&self->bus)))
		return err;

	return SUCCESS;
}

static void
axp803_regulator_release(const struct device *dev)
{
	const struct axp803_regulator *self = to_axp803_regulator(dev);

	axp803_put(&self->bus);
}

static const struct regulator_driver axp803_regulator_driver = {
	.drv = {
		.probe   = axp803_regulator_probe,
		.release = axp803_regulator_release,
	},
	.ops = {
		.get_info  = axp803_regulator_get_info,
		.get_state = axp803_regulator_get_state,
		.read_raw  = axp803_regulator_read_raw,
		.set_state = axp803_regulator_set_state,
		.write_raw = axp803_regulator_write_raw,
	},
};

const struct axp803_regulator axp803_regulator = {
	.dev = {
		.name  = "axp803-regulator",
		.drv   = &axp803_regulator_driver.drv,
		.state = DEVICE_STATE_INIT,
	},
	.bus = {
		.dev = &r_rsb.dev,
		.id  = AXP803_RSB_RTADDR,
	},
};
