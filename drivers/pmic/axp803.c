/*
 * Copyright © 2017-2020 The Crust Firmware Authors.
 * SPDX-License-Identifier: BSD-3-Clause OR GPL-2.0-only
 */

#include <regmap.h>
#include <util.h>
#include <mfd/axp20x.h>
#include <pmic/axp803.h>

#include "axp20x.h"

#define PIN_FUNCTION_REG 0x8f

static int
axp803_pmic_suspend(const struct device *dev)
{
	const struct axp20x_pmic *self = to_axp20x_pmic(dev);
	int err;

	/* Remember previous voltages when waking up from suspend. */
	if ((err = regmap_set_bits(self->map, PIN_FUNCTION_REG, BIT(1))))
		return err;

	/* Enable resume, allow IRQs during suspend. */
	return regmap_set_bits(self->map, WAKEUP_CTRL_REG, BIT(4) | BIT(3));
}

static const struct pmic_driver axp803_pmic_driver = {
	.drv = {
		.probe   = axp20x_pmic_probe,
		.release = axp20x_pmic_release,
	},
	.ops = {
		.reset    = axp20x_pmic_reset,
		.resume   = axp20x_pmic_resume,
		.shutdown = axp20x_pmic_shutdown,
		.suspend  = axp803_pmic_suspend,
	},
};

const struct axp20x_pmic axp803_pmic = {
	.dev = {
		.name  = "axp803-pmic",
		.drv   = &axp803_pmic_driver.drv,
		.state = DEVICE_STATE_INIT,
	},
	.map = &axp20x.map,
};
