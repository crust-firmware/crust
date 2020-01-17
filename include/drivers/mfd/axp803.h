/*
 * Copyright © 2017-2020 The Crust Firmware Authors.
 * SPDX-License-Identifier: BSD-3-Clause OR GPL-2.0-only
 */

#ifndef DRIVERS_MFD_AXP803_H
#define DRIVERS_MFD_AXP803_H

#include <device.h>
#include <regmap.h>

#define AXP803_RSB_RTADDR 0x2d

/**
 * Detect and initialize an AXP803 PMIC.
 *
 * This function may fail with:
 *  EIO    There was a problem communicating with the hardware.
 *  ENODEV No AXP803 PMIC was found.
 *
 * @param map  The device to check.
 * @return     Zero if an AXP803 is present and successfully initialized; any
 *             other value if it is not.
 */
int axp803_get(const struct regmap *map);

/**
 * Drop a reference to an AXP803 PMIC.
 */
void axp803_put(const struct regmap *map);

#endif /* DRIVERS_MFD_AXP803_H */
