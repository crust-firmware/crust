/*
 * Copyright © 2017-2022 The Crust Firmware Authors.
 * SPDX-License-Identifier: BSD-3-Clause OR GPL-2.0-only
 */

#ifndef DRIVERS_REGULATOR_SY8106A_H
#define DRIVERS_REGULATOR_SY8106A_H

#include <regmap.h>
#include <regulator.h>

#define SY8106A_I2C_ADDRESS 0x65

extern const struct regmap_device sy8106a;

#endif /* DRIVERS_REGULATOR_SY8106A_H */
