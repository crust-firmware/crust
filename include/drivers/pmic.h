/*
 * Copyright © 2017-2022 The Crust Firmware Authors.
 * SPDX-License-Identifier: BSD-3-Clause OR GPL-2.0-only
 */

#ifndef DRIVERS_PMIC_H
#define DRIVERS_PMIC_H

#include <device.h>

/**
 * Get a reference to an available PMIC device.
 *
 * @return A reference to a PMIC device.
 */
const struct device *pmic_get(void);

/**
 * Initiate the PMIC reset process.
 *
 * @param dev The device containing the PMIC functionality.
 */
int pmic_reset(const struct device *dev);

/**
 * Initiate the PMIC resume process.
 *
 * @param dev The device containing the PMIC functionality.
 */
int pmic_resume(const struct device *dev);

/**
 * Initiate the PMIC shutdown process.
 *
 * @param dev The device containing the PMIC functionality.
 */
int pmic_shutdown(const struct device *dev);

/**
 * Initiate the PMIC suspend process.
 *
 * @param dev The device containing the PMIC functionality.
 */
int pmic_suspend(const struct device *dev);

#endif /* DRIVERS_PMIC_H */
