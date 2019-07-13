/*
 * Copyright © 2017-2019 The Crust Firmware Authors.
 * SPDX-License-Identifier: BSD-3-Clause OR GPL-2.0-only
 */

#ifndef DRIVERS_PMIC_DUMMY_H
#define DRIVERS_PMIC_DUMMY_H

#include <dm.h>
#include <pmic.h>
#include <regulator.h>

struct dummy_pmic {
	struct device           dev;
	struct regulator_handle vdd_cpux;
};

extern struct dummy_pmic dummy_pmic;

#endif /* DRIVERS_PMIC_DUMMY_H */
