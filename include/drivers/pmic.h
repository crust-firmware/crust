/*
 * Copyright © 2017-2018 The Crust Firmware Authors.
 * SPDX-License-Identifier: BSD-3-Clause OR GPL-2.0-only
 */

#ifndef DRIVERS_PMIC_H
#define DRIVERS_PMIC_H

#include <dm.h>
#include <intrusive.h>

#define PMIC_OPS(dev) \
	(&container_of((dev)->drv, struct pmic_driver, drv)->ops)

struct pmic_driver_ops {
	int (*reset)(struct device *dev);
	int (*shutdown)(struct device *dev);
	int (*suspend)(struct device *dev);
	int (*wakeup)(struct device *dev);
};

struct pmic_driver {
	const struct driver          drv;
	const struct pmic_driver_ops ops;
};

/**
 * Initiate the PMIC reset process.
 *
 * @param dev The device containing the PMIC functionality.
 */
static inline int
pmic_reset(struct device *dev)
{
	return PMIC_OPS(dev)->reset(dev);
}

/**
 * Initiate the PMIC shutdown process.
 *
 * @param dev The device containing the PMIC functionality.
 */
static inline int
pmic_shutdown(struct device *dev)
{
	return PMIC_OPS(dev)->shutdown(dev);
}

/**
 * Initiate the PMIC suspend process.
 *
 * @param dev The device containing the PMIC functionality.
 */
static inline int
pmic_suspend(struct device *dev)
{
	return PMIC_OPS(dev)->suspend(dev);
}

/**
 * Initiate the PMIC wakeup process.
 *
 * @param dev The device containing the PMIC functionality.
 */
static inline int
pmic_wakeup(struct device *dev)
{
	return PMIC_OPS(dev)->wakeup(dev);
}

#endif /* DRIVERS_PMIC_H */
