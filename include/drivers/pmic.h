/*
 * Copyright © 2017-2018 The Crust Firmware Authors.
 * SPDX-License-Identifier: (BSD-3-Clause OR GPL-2.0)
 */

#ifndef DRIVERS_PMIC_H
#define DRIVERS_PMIC_H

#include <dm.h>
#include <intrusive.h>

#define PMIC_OPS(dev) \
	(&container_of((dev)->drv, struct pmic_driver, drv)->ops)

struct pmic_driver_ops {
	void (*shutdown)(struct device *dev);
	void (*suspend)(struct device *dev);
};

struct pmic_driver {
	const struct driver          drv;
	const struct pmic_driver_ops ops;
};

/**
 * Initiates the PMIC shutdown process.
 *
 * @param dev   The device containing the PMIC functionality.
 */
static inline void
pmic_shutdown(struct device *dev)
{
	PMIC_OPS(dev)->shutdown(dev);
}

/**
 * Initiates the PMIC suspend process.
 *
 * @param dev   The device containing the PMIC functionality.
 */
static inline void
pmic_suspend(struct device *dev)
{
	PMIC_OPS(dev)->suspend(dev);
}

#endif /* DRIVERS_PMIC_H */
