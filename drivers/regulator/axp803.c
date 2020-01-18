/*
 * Copyright © 2017-2020 The Crust Firmware Authors.
 * SPDX-License-Identifier: BSD-3-Clause OR GPL-2.0-only
 */

#include <error.h>
#include <regulator.h>
#include <mfd/axp803.h>
#include <regulator/axp803.h>

#include "regulator.h"

#define OUTPUT_POWER_CONTROL1 0x10
#define OUTPUT_POWER_CONTROL2 0x12
#define OUTPUT_POWER_CONTROL3 0x13

#define GPIO_LDO_ON           0x3
#define GPIO_LDO_OFF          0x4

struct axp803_regulator_info {
	struct regulator_info info;
	uint8_t               enable_register;
	uint8_t               enable_mask;
	uint8_t               value_register;
	uint8_t               value_mask;
};

static const struct axp803_regulator_info axp803_regulators[] = {
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
		.enable_register = OUTPUT_POWER_CONTROL1,
		.enable_mask     = BIT(0),
		.value_register  = 0x20,
		.value_mask      = GENMASK(4, 0),
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
		.enable_register = OUTPUT_POWER_CONTROL1,
		.enable_mask     = BIT(1),
		.value_register  = 0x21,
		.value_mask      = GENMASK(6, 0),
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
		.enable_register = OUTPUT_POWER_CONTROL1,
		.enable_mask     = BIT(2),
		.value_register  = 0x22,
		.value_mask      = GENMASK(6, 0),
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
		.enable_register = OUTPUT_POWER_CONTROL1,
		.enable_mask     = BIT(3),
		.value_register  = 0x23,
		.value_mask      = GENMASK(6, 0),
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
		.enable_register = OUTPUT_POWER_CONTROL1,
		.enable_mask     = BIT(4),
		.value_register  = 0x24,
		.value_mask      = GENMASK(6, 0),
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
		.enable_register = OUTPUT_POWER_CONTROL1,
		.enable_mask     = BIT(5),
		.value_register  = 0x25,
		.value_mask      = GENMASK(6, 0),
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
		.enable_register = OUTPUT_POWER_CONTROL2,
		.enable_mask     = BIT(7),
		.value_register  = 0x20,
		.value_mask      = GENMASK(4, 0),
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
		.enable_register = OUTPUT_POWER_CONTROL3,
		.enable_mask     = BIT(5),
		.value_register  = 0x28,
		.value_mask      = GENMASK(4, 0),
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
		.enable_register = OUTPUT_POWER_CONTROL3,
		.enable_mask     = BIT(6),
		.value_register  = 0x29,
		.value_mask      = GENMASK(4, 0),
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
		.enable_register = OUTPUT_POWER_CONTROL3,
		.enable_mask     = BIT(7),
		.value_register  = 0x2a,
		.value_mask      = GENMASK(4, 0),
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
		.enable_register = OUTPUT_POWER_CONTROL2,
		.enable_mask     = BIT(3),
		.value_register  = 0x15,
		.value_mask      = GENMASK(4, 0),
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
		.enable_register = OUTPUT_POWER_CONTROL2,
		.enable_mask     = BIT(4),
		.value_register  = 0x16,
		.value_mask      = GENMASK(4, 0),
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
		.enable_register = OUTPUT_POWER_CONTROL2,
		.enable_mask     = BIT(5),
		.value_register  = 0x17,
		.value_mask      = GENMASK(4, 0),
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
		.enable_register = OUTPUT_POWER_CONTROL2,
		.enable_mask     = BIT(6),
		.value_register  = 0x18,
		.value_mask      = GENMASK(4, 0),
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
		.enable_register = OUTPUT_POWER_CONTROL2,
		.enable_mask     = BIT(0),
		.value_register  = 0x19,
		.value_mask      = GENMASK(4, 0),
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
		.enable_register = OUTPUT_POWER_CONTROL2,
		.enable_mask     = BIT(1),
		.value_register  = 0x1a,
		.value_mask      = GENMASK(4, 0),
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
		.enable_register = OUTPUT_POWER_CONTROL2,
		.enable_mask     = BIT(2),
		.value_register  = 0x1b,
		.value_mask      = GENMASK(4, 0),
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
		.enable_register = OUTPUT_POWER_CONTROL3,
		.enable_mask     = BIT(2),
		.value_register  = 0x1c,
		.value_mask      = GENMASK(3, 0),
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
		.enable_register = OUTPUT_POWER_CONTROL3,
		.enable_mask     = BIT(3),
		.value_register  = 0x1d,
		.value_mask      = GENMASK(3, 0),
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
		.enable_register = 0x90,
		.enable_mask     = GENMASK(2, 0),
		.value_register  = 0x91,
		.value_mask      = GENMASK(4, 0),
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
		.enable_register = 0x92,
		.enable_mask     = GENMASK(2, 0),
		.value_register  = 0x93,
		.value_mask      = GENMASK(4, 0),
	},
};

static const struct regulator_info *
axp803_regulator_get_info(const struct device *dev UNUSED, uint8_t id)
{
	return &axp803_regulators[id].info;
}

static int
axp803_regulator_get_state(const struct device *dev UNUSED, uint8_t id)
{
	const struct regmap *map = &axp803.map;
	uint8_t reg  = axp803_regulators[id].enable_register;
	uint8_t mask = axp803_regulators[id].enable_mask;
	uint8_t val;
	int err;

	if ((err = regmap_read(map, reg, &val)))
		return err;

	/* GPIO LDOs have a pin function, not an enable bit. */
	if (id >= AXP803_REGL_GPIO0)
		return (val & mask) == GPIO_LDO_ON;

	return !!(val & mask);
}

static int
axp803_regulator_read_raw(const struct device *dev UNUSED, uint8_t id,
                          uint32_t *raw)
{
	const struct regmap *map = &axp803.map;
	uint8_t reg  = axp803_regulators[id].value_register;
	uint8_t mask = axp803_regulators[id].value_mask;
	uint8_t val;
	int err;

	if ((err = regmap_read(map, reg, &val)))
		return err;

	*raw = val & mask;

	return SUCCESS;
}

static int
axp803_regulator_set_state(const struct device *dev UNUSED, uint8_t id,
                           bool enabled)
{
	const struct regmap *map = &axp803.map;
	uint8_t reg  = axp803_regulators[id].enable_register;
	uint8_t mask = axp803_regulators[id].enable_mask;
	uint8_t val;

	/* GPIO LDOs have a pin function, not an enable bit. */
	if (id >= AXP803_REGL_GPIO0)
		val = enabled ? GPIO_LDO_ON : GPIO_LDO_OFF;
	else
		val = enabled ? mask : 0;

	return regmap_update_bits(map, reg, mask, val);
}

static int
axp803_regulator_write_raw(const struct device *dev UNUSED, uint8_t id,
                           uint32_t raw)
{
	const struct regmap *map = &axp803.map;
	uint8_t reg  = axp803_regulators[id].value_register;
	uint8_t mask = axp803_regulators[id].value_mask;

	/* AXP803_REGL_DC1SW is a secondary output of AXP803_REGL_DCDC1,
	 * without its own voltage control. Only pretend to set its voltage. */
	if (id == AXP803_REGL_DC1SW)
		return SUCCESS;

	return regmap_update_bits(map, reg, mask, raw);
}

static const struct regulator_driver axp803_regulator_driver = {
	.drv = {
		.probe   = axp803_subdevice_probe,
		.release = axp803_subdevice_release,
	},
	.ops = {
		.get_info  = axp803_regulator_get_info,
		.get_state = axp803_regulator_get_state,
		.read_raw  = axp803_regulator_read_raw,
		.set_state = axp803_regulator_set_state,
		.write_raw = axp803_regulator_write_raw,
	},
};

const struct device axp803_regulator = {
	.name  = "axp803-regulator",
	.drv   = &axp803_regulator_driver.drv,
	.state = DEVICE_STATE_INIT,
};
