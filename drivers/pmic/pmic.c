/*
 * Copyright © 2017-2020 The Crust Firmware Authors.
 * SPDX-License-Identifier: BSD-3-Clause OR GPL-2.0-only
 */

#include <device.h>
#include <intrusive.h>
#include <pmic.h>
#include <pmic/axp803.h>
#include <pmic/axp805.h>

#include "pmic.h"

/**
 * Get the ops for the pmic controller device.
 */
static inline const struct pmic_driver_ops *
pmic_ops_for(const struct device *dev)
{
	const struct pmic_driver *drv =
		container_of(dev->drv, const struct pmic_driver, drv);

	return &drv->ops;
}

const struct device *
pmic_get(void)
{
	const struct device *pmic = NULL;

	if (CONFIG(PMIC_AXP803))
		pmic = device_get_or_null(&axp803_pmic.dev);
	if (!pmic && CONFIG(PMIC_AXP805))
		pmic = device_get_or_null(&axp805_pmic.dev);

	return pmic;
}

int
pmic_reset(const struct device *dev)
{
	return pmic_ops_for(dev)->reset(dev);
}

int
pmic_resume(const struct device *dev)
{
	return pmic_ops_for(dev)->resume(dev);
}

int
pmic_shutdown(const struct device *dev)
{
	return pmic_ops_for(dev)->shutdown(dev);
}

int
pmic_suspend(const struct device *dev)
{
	return pmic_ops_for(dev)->suspend(dev);
}
