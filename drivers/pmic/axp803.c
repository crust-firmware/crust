/*
 * Copyright © 2017-2020 The Crust Firmware Authors.
 * SPDX-License-Identifier: BSD-3-Clause OR GPL-2.0-only
 */

#include <regmap.h>
#include <util.h>
#include <mfd/axp20x.h>
#include <pmic/axp803.h>

#include "pmic.h"

#define WAKEUP_CTRL_REG   0x31
#define POWER_DISABLE_REG 0x32
#define PIN_FUNCTION_REG  0x8f

static inline const struct axp803_pmic *
to_axp803_pmic(const struct device *dev)
{
	return container_of(dev, const struct axp803_pmic, dev);
}

static int
axp803_pmic_reset(const struct device *dev)
{
	const struct axp803_pmic *self = to_axp803_pmic(dev);

	/* Trigger soft power restart. */
	return regmap_set_bits(self->map, WAKEUP_CTRL_REG, BIT(6));
}

static int
axp803_pmic_resume(const struct device *dev)
{
	const struct axp803_pmic *self = to_axp803_pmic(dev);

	/* Trigger soft power resume. */
	return regmap_set_bits(self->map, WAKEUP_CTRL_REG, BIT(5));
}

static int
axp803_pmic_shutdown(const struct device *dev)
{
	const struct axp803_pmic *self = to_axp803_pmic(dev);

	/* Trigger soft power off. */
	return regmap_set_bits(self->map, POWER_DISABLE_REG, BIT(7));
}

static int
axp803_pmic_suspend(const struct device *dev)
{
	const struct axp803_pmic *self = to_axp803_pmic(dev);
	int err;

	/* Remember previous voltages when waking up from suspend. */
	if ((err = regmap_set_bits(self->map, PIN_FUNCTION_REG, BIT(1))))
		return err;

	/* Enable resume, allow IRQs during suspend. */
	return regmap_set_bits(self->map, WAKEUP_CTRL_REG, BIT(4) | BIT(3));
}

static int
axp803_pmic_probe(const struct device *dev)
{
	const struct axp803_pmic *self = to_axp803_pmic(dev);

	return regmap_user_probe(self->map);
}

static void
axp803_pmic_release(const struct device *dev)
{
	const struct axp803_pmic *self = to_axp803_pmic(dev);

	regmap_user_release(self->map);
}

static const struct pmic_driver axp803_pmic_driver = {
	.drv = {
		.probe   = axp803_pmic_probe,
		.release = axp803_pmic_release,
	},
	.ops = {
		.reset    = axp803_pmic_reset,
		.resume   = axp803_pmic_resume,
		.shutdown = axp803_pmic_shutdown,
		.suspend  = axp803_pmic_suspend,
	},
};

const struct axp803_pmic axp803_pmic = {
	.dev = {
		.name  = "axp803-pmic",
		.drv   = &axp803_pmic_driver.drv,
		.state = DEVICE_STATE_INIT,
	},
	.map = &axp20x.map,
};
