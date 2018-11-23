/*
 * Copyright © 2017-2018 The Crust Firmware Authors.
 * SPDX-License-Identifier: BSD-3-Clause OR GPL-2.0-only
 */

#include <compiler.h>
#include <css.h>
#include <debug.h>
#include <delay.h>
#include <dm.h>
#include <error.h>
#include <interrupts.h>
#include <pmic.h>
#include <regulator.h>
#include <stdbool.h>
#include <stddef.h>
#include <system_power.h>
#include <watchdog.h>

static bool is_off;
static bool is_suspended;

static void
disable_regulators(void)
{
	struct device *dev;
	int     err;
	uint8_t id;

	/* Disable all external voltage regulators. */
	for (dev = dm_get_subdev_by_index(DM_CLASS_REGULATOR, 0, &id);
	     dev != NULL; dev = dm_next_subdev(dev, &id)) {
		if ((err = regulator_disable(dev, id)) && err != EPERM) {
			const char *name = regulator_get_info(dev, id)->name;
			warn("Failed to turn off regulator %s.%s (%d)",
			     dev->name, name, err);
		}
	}
}

bool __pure
system_is_off(void)
{
	return is_off;
}

bool __pure
system_is_suspended(void)
{
	return is_suspended;
}

noreturn void
system_reset(void)
{
	struct device *pmic, *watchdog;

	if ((pmic = dm_first_dev_by_class(DM_CLASS_PMIC)))
		pmic_reset(pmic);

	if ((watchdog = dm_first_dev_by_class(DM_CLASS_WATCHDOG))) {
		watchdog_disable(watchdog);
		watchdog_enable(watchdog, 0);
		/* This is always at least one reference clock cycle. */
		udelay(1);
	}

	panic("Failed to reset system");
}

void
system_shutdown(void)
{
	struct device *pmic;

	disable_regulators();

	if ((pmic = dm_first_dev_by_class(DM_CLASS_PMIC)))
		pmic_shutdown(pmic);

	/* We didn't actually shut down. Wait for a wakeup event and reset. */
	is_off = true;
}

void
system_suspend(void)
{
	struct device *pmic;
	uint32_t flags = disable_interrupts();

	/* If the system is already suspended, do nothing. */
	if (is_suspended)
		return;

	/* Mark the system as being suspended. */
	is_suspended = true;

	/* State management is done, so we can resume handling interrupts. */
	restore_interrupts(flags);

	if ((pmic = dm_first_dev_by_class(DM_CLASS_PMIC)))
		pmic_suspend(pmic);
}

void
system_wakeup(void)
{
	struct device *pmic;
	uint32_t flags = disable_interrupts();

	/* Tried to wake up from a fake "off" state. Reset the system. */
	if (is_off)
		system_reset();
	/* If the system is already awake, do nothing. */
	if (!is_suspended)
		return;

	/* Mark the system as no longer being suspended. */
	is_suspended = false;

	/* State management is done, so we can resume handling interrupts. */
	restore_interrupts(flags);

	/* Tell the PMIC to turn everything back on. */
	if ((pmic = dm_first_dev_by_class(DM_CLASS_PMIC)))
		pmic_wakeup(pmic);

	/* Resume execution on the CSS. */
	css_set_css_state(POWER_STATE_ON);
	css_set_cluster_state(0, POWER_STATE_ON);
	css_set_core_state(0, 0, POWER_STATE_ON);
}
