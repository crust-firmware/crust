/*
 * Copyright © 2017-2020 The Crust Firmware Authors.
 * SPDX-License-Identifier: BSD-3-Clause OR GPL-2.0-only
 */

#include <regmap.h>
#include <util.h>
#include <mfd/axp20x.h>
#include <pmic/axp805.h>

#include "axp20x.h"

static int
axp805_pmic_reset(const struct device *dev)
{
	const struct axp20x_pmic *self = to_axp20x_pmic(dev);

	/* Trigger soft power restart. */
	return regmap_set_bits(self->map, POWER_DISABLE_REG, BIT(6));
}

static int
axp805_pmic_suspend(const struct device *dev)
{
	const struct axp20x_pmic *self = to_axp20x_pmic(dev);

	/* Enable resume, remember voltages, and allow IRQs during suspend. */
	return regmap_set_bits(self->map, WAKEUP_CTRL_REG,
	                       BIT(6) | BIT(4) | BIT(3));
}

static const struct pmic_driver axp805_pmic_driver = {
	.drv = {
		.probe   = axp20x_pmic_probe,
		.release = axp20x_pmic_release,
	},
	.ops = {
		.reset    = axp805_pmic_reset,
		.resume   = axp20x_pmic_resume,
		.shutdown = axp20x_pmic_shutdown,
		.suspend  = axp805_pmic_suspend,
	},
};

const struct axp20x_pmic axp805_pmic = {
	.dev = {
		.name  = "axp805-pmic",
		.drv   = &axp805_pmic_driver.drv,
		.state = DEVICE_STATE_INIT,
	},
	.map = &axp20x.map,
};
