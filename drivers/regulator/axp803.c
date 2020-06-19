/*
 * Copyright © 2017-2020 The Crust Firmware Authors.
 * SPDX-License-Identifier: BSD-3-Clause OR GPL-2.0-only
 */

#include <util.h>
#include <mfd/axp20x.h>
#include <regulator/axp20x.h>
#include <regulator/axp803.h>

#include "axp20x.h"

#define OUTPUT_POWER_CONTROL1 0x10
#define OUTPUT_POWER_CONTROL2 0x12
#define OUTPUT_POWER_CONTROL3 0x13

static const struct axp20x_regulator_info axp803_regulators[] = {
	[AXP803_REGL_DCDC1] = {
		.enable_register = OUTPUT_POWER_CONTROL1,
		.enable_mask     = BIT(0),
		.value_register  = 0x20,
		.value_mask      = GENMASK(4, 0),
	},
	[AXP803_REGL_DCDC2] = {
		.enable_register = OUTPUT_POWER_CONTROL1,
		.enable_mask     = BIT(1),
		.value_register  = 0x21,
		.value_mask      = GENMASK(6, 0),
	},
	[AXP803_REGL_DCDC3] = {
		.enable_register = OUTPUT_POWER_CONTROL1,
		.enable_mask     = BIT(2),
		.value_register  = 0x22,
		.value_mask      = GENMASK(6, 0),
	},
	[AXP803_REGL_DCDC4] = {
		.enable_register = OUTPUT_POWER_CONTROL1,
		.enable_mask     = BIT(3),
		.value_register  = 0x23,
		.value_mask      = GENMASK(6, 0),
	},
	[AXP803_REGL_DCDC5] = {
		.enable_register = OUTPUT_POWER_CONTROL1,
		.enable_mask     = BIT(4),
		.value_register  = 0x24,
		.value_mask      = GENMASK(6, 0),
	},
	[AXP803_REGL_DCDC6] = {
		.enable_register = OUTPUT_POWER_CONTROL1,
		.enable_mask     = BIT(5),
		.value_register  = 0x25,
		.value_mask      = GENMASK(6, 0),
	},
	[AXP803_REGL_DC1SW] = {
		.enable_register = OUTPUT_POWER_CONTROL2,
		.enable_mask     = BIT(7),
		.value_register  = 0x20,
		.value_mask      = GENMASK(4, 0),
	},
	[AXP803_REGL_ALDO1] = {
		.enable_register = OUTPUT_POWER_CONTROL3,
		.enable_mask     = BIT(5),
		.value_register  = 0x28,
		.value_mask      = GENMASK(4, 0),
	},
	[AXP803_REGL_ALDO2] = {
		.enable_register = OUTPUT_POWER_CONTROL3,
		.enable_mask     = BIT(6),
		.value_register  = 0x29,
		.value_mask      = GENMASK(4, 0),
	},
	[AXP803_REGL_ALDO3] = {
		.enable_register = OUTPUT_POWER_CONTROL3,
		.enable_mask     = BIT(7),
		.value_register  = 0x2a,
		.value_mask      = GENMASK(4, 0),
	},
	[AXP803_REGL_DLDO1] = {
		.enable_register = OUTPUT_POWER_CONTROL2,
		.enable_mask     = BIT(3),
		.value_register  = 0x15,
		.value_mask      = GENMASK(4, 0),
	},
	[AXP803_REGL_DLDO2] = {
		.enable_register = OUTPUT_POWER_CONTROL2,
		.enable_mask     = BIT(4),
		.value_register  = 0x16,
		.value_mask      = GENMASK(4, 0),
	},
	[AXP803_REGL_DLDO3] = {
		.enable_register = OUTPUT_POWER_CONTROL2,
		.enable_mask     = BIT(5),
		.value_register  = 0x17,
		.value_mask      = GENMASK(4, 0),
	},
	[AXP803_REGL_DLDO4] = {
		.enable_register = OUTPUT_POWER_CONTROL2,
		.enable_mask     = BIT(6),
		.value_register  = 0x18,
		.value_mask      = GENMASK(4, 0),
	},
	[AXP803_REGL_ELDO1] = {
		.enable_register = OUTPUT_POWER_CONTROL2,
		.enable_mask     = BIT(0),
		.value_register  = 0x19,
		.value_mask      = GENMASK(4, 0),
	},
	[AXP803_REGL_ELDO2] = {
		.enable_register = OUTPUT_POWER_CONTROL2,
		.enable_mask     = BIT(1),
		.value_register  = 0x1a,
		.value_mask      = GENMASK(4, 0),
	},
	[AXP803_REGL_ELDO3] = {
		.enable_register = OUTPUT_POWER_CONTROL2,
		.enable_mask     = BIT(2),
		.value_register  = 0x1b,
		.value_mask      = GENMASK(4, 0),
	},
	[AXP803_REGL_FLDO1] = {
		.enable_register = OUTPUT_POWER_CONTROL3,
		.enable_mask     = BIT(2),
		.value_register  = 0x1c,
		.value_mask      = GENMASK(3, 0),
	},
	[AXP803_REGL_FLDO2] = {
		.enable_register = OUTPUT_POWER_CONTROL3,
		.enable_mask     = BIT(3),
		.value_register  = 0x1d,
		.value_mask      = GENMASK(3, 0),
	},
	[AXP803_REGL_GPIO0] = {
		.enable_register = 0x90,
		.enable_mask     = GENMASK(2, 0),
		.value_register  = 0x91,
		.value_mask      = GENMASK(4, 0),
	},
	[AXP803_REGL_GPIO1] = {
		.enable_register = 0x92,
		.enable_mask     = GENMASK(2, 0),
		.value_register  = 0x93,
		.value_mask      = GENMASK(4, 0),
	},
};

const struct axp20x_regulator axp803_regulator = {
	.dev = {
		.name  = "axp803-regulator",
		.drv   = &axp20x_regulator_driver.drv,
		.state = DEVICE_STATE_INIT,
	},
	.map  = &axp20x.map,
	.info = axp803_regulators,
};
