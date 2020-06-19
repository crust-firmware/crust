/*
 * Copyright © 2017-2020 The Crust Firmware Authors.
 * SPDX-License-Identifier: BSD-3-Clause OR GPL-2.0-only
 */

#include <regmap.h>
#include <util.h>

#include "axp20x.h"

int
axp20x_pmic_reset(const struct device *dev)
{
	const struct axp20x_pmic *self = to_axp20x_pmic(dev);

	/* Trigger soft power restart. */
	return regmap_set_bits(self->map, WAKEUP_CTRL_REG, BIT(6));
}

int
axp20x_pmic_resume(const struct device *dev)
{
	const struct axp20x_pmic *self = to_axp20x_pmic(dev);

	/* Trigger soft power resume. */
	return regmap_set_bits(self->map, WAKEUP_CTRL_REG, BIT(5));
}

int
axp20x_pmic_shutdown(const struct device *dev)
{
	const struct axp20x_pmic *self = to_axp20x_pmic(dev);

	/* Trigger soft power off. */
	return regmap_set_bits(self->map, POWER_DISABLE_REG, BIT(7));
}

int
axp20x_pmic_probe(const struct device *dev)
{
	const struct axp20x_pmic *self = to_axp20x_pmic(dev);

	return regmap_user_probe(self->map);
}

void
axp20x_pmic_release(const struct device *dev)
{
	const struct axp20x_pmic *self = to_axp20x_pmic(dev);

	regmap_user_release(self->map);
}
