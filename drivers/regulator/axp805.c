/*
 * Copyright © 2017-2020 The Crust Firmware Authors.
 * SPDX-License-Identifier: BSD-3-Clause OR GPL-2.0-only
 */

#include <util.h>
#include <mfd/axp20x.h>
#include <regulator/axp20x.h>
#include <regulator/axp805.h>

#include "axp20x.h"

#define POWER_ONOFF_CTRL_REG1 0x10
#define POWER_ONOFF_CTRL_REG2 0x11

static const struct axp20x_regulator_info axp805_regulators[] = {
	[AXP805_REGL_DCDCA] = {
		.enable_register = POWER_ONOFF_CTRL_REG1,
		.enable_mask     = BIT(0),
		.value_register  = 0x12,
		.value_mask      = GENMASK(6, 0),
	},
	[AXP805_REGL_DCDCB] = {
		.enable_register = POWER_ONOFF_CTRL_REG1,
		.enable_mask     = BIT(1),
		.value_register  = 0x13,
		.value_mask      = GENMASK(4, 0),
	},
	[AXP805_REGL_DCDCC] = {
		.enable_register = POWER_ONOFF_CTRL_REG1,
		.enable_mask     = BIT(2),
		.value_register  = 0x14,
		.value_mask      = GENMASK(6, 0),
	},
	[AXP805_REGL_DCDCD] = {
		.enable_register = POWER_ONOFF_CTRL_REG1,
		.enable_mask     = BIT(3),
		.value_register  = 0x15,
		.value_mask      = GENMASK(5, 0),
	},
	[AXP805_REGL_DCDCE] = {
		.enable_register = POWER_ONOFF_CTRL_REG1,
		.enable_mask     = BIT(4),
		.value_register  = 0x16,
		.value_mask      = GENMASK(4, 0),
	},
	[AXP805_REGL_ALDO1] = {
		.enable_register = POWER_ONOFF_CTRL_REG1,
		.enable_mask     = BIT(5),
		.value_register  = 0x17,
		.value_mask      = GENMASK(4, 0),
	},
	[AXP805_REGL_ALDO2] = {
		.enable_register = POWER_ONOFF_CTRL_REG1,
		.enable_mask     = BIT(6),
		.value_register  = 0x18,
		.value_mask      = GENMASK(4, 0),
	},
	[AXP805_REGL_ALDO3] = {
		.enable_register = POWER_ONOFF_CTRL_REG1,
		.enable_mask     = BIT(7),
		.value_register  = 0x19,
		.value_mask      = GENMASK(4, 0),
	},
	[AXP805_REGL_BLDO1] = {
		.enable_register = POWER_ONOFF_CTRL_REG2,
		.enable_mask     = BIT(0),
		.value_register  = 0x20,
		.value_mask      = GENMASK(3, 0),
	},
	[AXP805_REGL_BLDO2] = {
		.enable_register = POWER_ONOFF_CTRL_REG2,
		.enable_mask     = BIT(1),
		.value_register  = 0x21,
		.value_mask      = GENMASK(3, 0),
	},
	[AXP805_REGL_BLDO3] = {
		.enable_register = POWER_ONOFF_CTRL_REG2,
		.enable_mask     = BIT(2),
		.value_register  = 0x22,
		.value_mask      = GENMASK(3, 0),
	},
	[AXP805_REGL_BLDO4] = {
		.enable_register = POWER_ONOFF_CTRL_REG2,
		.enable_mask     = BIT(3),
		.value_register  = 0x23,
		.value_mask      = GENMASK(3, 0),
	},
	[AXP805_REGL_CLDO1] = {
		.enable_register = POWER_ONOFF_CTRL_REG2,
		.enable_mask     = BIT(4),
		.value_register  = 0x24,
		.value_mask      = GENMASK(4, 0),
	},
	[AXP805_REGL_CLDO2] = {
		.enable_register = POWER_ONOFF_CTRL_REG2,
		.enable_mask     = BIT(5),
		.value_register  = 0x25,
		.value_mask      = GENMASK(4, 0),
	},
	[AXP805_REGL_CLDO3] = {
		.enable_register = POWER_ONOFF_CTRL_REG2,
		.enable_mask     = BIT(6),
		.value_register  = 0x26,
		.value_mask      = GENMASK(4, 0),
	},
	[AXP805_REGL_DCSW] = {
		.enable_register = POWER_ONOFF_CTRL_REG2,
		.enable_mask     = BIT(7),
	},
};

const struct axp20x_regulator axp805_regulator = {
	.dev = {
		.name  = "axp805-regulator",
		.drv   = &axp20x_regulator_driver.drv,
		.state = DEVICE_STATE_INIT,
	},
	.map  = &axp20x.map,
	.info = axp805_regulators,
};
