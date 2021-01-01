/*
 * Copyright © 2017-2021 The Crust Firmware Authors.
 * SPDX-License-Identifier: BSD-3-Clause OR GPL-2.0-only
 */

#ifndef PMIC_PRIVATE_H
#define PMIC_PRIVATE_H

#include <device.h>
#include <pmic.h>

struct pmic_driver_ops {
	int (*reset)(const struct device *dev);
	int (*resume)(const struct device *dev);
	int (*shutdown)(const struct device *dev);
	int (*suspend)(const struct device *dev);
};

struct pmic_driver {
	struct driver          drv;
	struct pmic_driver_ops ops;
};

#endif /* PMIC_PRIVATE_H */
