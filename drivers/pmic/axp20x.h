/*
 * Copyright © 2017-2020 The Crust Firmware Authors.
 * SPDX-License-Identifier: BSD-3-Clause OR GPL-2.0-only
 */

#ifndef AXP20X_PRIVATE_H
#define AXP20X_PRIVATE_H

#include <intrusive.h>
#include <pmic/axp20x.h>

#include "pmic.h"

#define WAKEUP_CTRL_REG   0x31
#define POWER_DISABLE_REG 0x32

static inline const struct axp20x_pmic *
to_axp20x_pmic(const struct device *dev)
{
	return container_of(dev, const struct axp20x_pmic, dev);
}

/* Valid for AXP221, AXP223, AXP803. */
int axp20x_pmic_reset(const struct device *dev);
/* Valid for AXP221, AXP223, AXP803, AXP805, AXP813. */
int axp20x_pmic_resume(const struct device *dev);
/* Valid for AXP221, AXP223, AXP803, AXP805, AXP813. */
int axp20x_pmic_shutdown(const struct device *dev);

int axp20x_pmic_probe(const struct device *dev);
void axp20x_pmic_release(const struct device *dev);

#endif /* AXP20X_PRIVATE_H */
