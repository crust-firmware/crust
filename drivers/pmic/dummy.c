/*
 * Copyright © 2017-2019 The Crust Firmware Authors.
 * SPDX-License-Identifier: BSD-3-Clause OR GPL-2.0-only
 */

#include <compiler.h>
#include <dm.h>
#include <error.h>
#include <pmic.h>
#include <regulator.h>
#include <pmic/dummy.h>
#include <regulator/sy8106a.h>

static int
dummy_pmic_power_off(struct device *dev)
{
	if (!dev->supplydev)
		return SUCCESS;

	/* Turn CPU power off. */
	return regulator_disable(dev->supplydev, dev->supply);
}

static int
dummy_pmic_power_on(struct device *dev)
{
	if (!dev->supplydev)
		return SUCCESS;

	/* Turn CPU power on. */
	return regulator_enable(dev->supplydev, dev->supply);
}

static int
dummy_pmic_probe(struct device *dev __unused)
{
	return SUCCESS;
}

static const struct pmic_driver dummy_pmic_driver = {
	.drv = {
		.class = DM_CLASS_PMIC,
		.probe = dummy_pmic_probe,
	},
	.ops = {
		.reset    = dummy_pmic_power_on,
		.shutdown = dummy_pmic_power_off,
		.suspend  = dummy_pmic_power_off,
		.wakeup   = dummy_pmic_power_on,
	},
};

struct device dummy_pmic = {
	.name = "dummy-pmic",
	.drv  = &dummy_pmic_driver.drv,
#if CONFIG_REGULATOR_SY8106A
	.supplydev = &sy8106a,
	.supply    = SY8106A_REGL_VOUT,
#endif
};
