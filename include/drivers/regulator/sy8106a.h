/*
 * Copyright © 2017-2018 The Crust Firmware Authors.
 * SPDX-License-Identifier: (BSD-3-Clause OR GPL-2.0)
 */

#ifndef DRIVERS_REGULATOR_SY8106A_H
#define DRIVERS_REGULATOR_SY8106A_H

#include <regulator.h>

#define SY8106A_I2C_ADDRESS 0x65

extern const struct regulator_driver sy8106a_driver;

#endif /* DRIVERS_REGULATOR_SY8106A_H */
