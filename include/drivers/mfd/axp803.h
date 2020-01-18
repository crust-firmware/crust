/*
 * Copyright © 2017-2020 The Crust Firmware Authors.
 * SPDX-License-Identifier: BSD-3-Clause OR GPL-2.0-only
 */

#ifndef DRIVERS_MFD_AXP803_H
#define DRIVERS_MFD_AXP803_H

#include <device.h>
#include <regmap.h>

struct axp803 {
	struct device dev;
	struct regmap map;
};

extern const struct axp803 axp803;

int axp803_subdevice_probe(const struct device *dev);

void axp803_subdevice_release(const struct device *dev);

#endif /* DRIVERS_MFD_AXP803_H */
