/*
 * Copyright © 2017-2018 The Crust Firmware Authors.
 * SPDX-License-Identifier: (BSD-3-Clause OR GPL-2.0)
 */

#include <clock.h>
#include <compiler.h>
#include <debug.h>
#include <delay.h>
#include <dm.h>
#include <pmic.h>
#include <prcm.h>
#include <regulator.h>
#include <stddef.h>
#include <watchdog.h>

noreturn void
system_reset(void)
{
	struct device *watchdog = dm_first_dev_by_class(DM_CLASS_WATCHDOG);

	if (watchdog != NULL) {
		watchdog_disable(watchdog);
		watchdog_enable(watchdog, 0);
		udelay(1);
	}
	panic("Failed to reset system");
}

void
system_shutdown(void)
{
	uint8_t id;
	struct device *pmic, *regl;

	/* Disable voltage regulators for each regulator driver. */
	for (regl = dm_get_subdev_by_index(DM_CLASS_REGULATOR, 0, &id);
	     regl != NULL; regl = dm_next_subdev(regl, &id))
		regulator_disable(regl, id);

	prcm_shutdown();

	if ((pmic = dm_first_dev_by_class(DM_CLASS_PMIC)))
		pmic_shutdown(pmic);
}

void
system_suspend(void)
{
	struct device *pmic;

	if ((pmic = dm_first_dev_by_class(DM_CLASS_PMIC)))
		pmic_suspend(pmic);
}
