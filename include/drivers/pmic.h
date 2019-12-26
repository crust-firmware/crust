/*
 * Copyright © 2017-2019 The Crust Firmware Authors.
 * SPDX-License-Identifier: BSD-3-Clause OR GPL-2.0-only
 */

#ifndef DRIVERS_PMIC_H
#define DRIVERS_PMIC_H

#include <device.h>
#include <intrusive.h>

#define PMIC_OPS(dev) \
	(&container_of((dev)->drv, const struct pmic_driver, drv)->ops)

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
static inline int
pmic_reset(const struct device *dev)
{
	return PMIC_OPS(dev)->reset(dev);
}

/**
 * Initiate the PMIC resume process.
 *
 * @param dev The device containing the PMIC functionality.
 */
static inline int
pmic_resume(const struct device *dev)
{
	return PMIC_OPS(dev)->resume(dev);
}

/**
 * Initiate the PMIC shutdown process.
 *
 * @param dev The device containing the PMIC functionality.
 */
static inline int
pmic_shutdown(const struct device *dev)
{
	return PMIC_OPS(dev)->shutdown(dev);
}

/**
 * Initiate the PMIC suspend process.
 *
 * @param dev The device containing the PMIC functionality.
 */
static inline int
pmic_suspend(const struct device *dev)
{
	return PMIC_OPS(dev)->suspend(dev);
}

#endif /* DRIVERS_PMIC_H */
