/*
 * Copyright © 2017-2018 The Crust Firmware Authors.
 * SPDX-License-Identifier: (BSD-3-Clause OR GPL-2.0)
 */

#include <compiler.h>
#include <debug.h>
#include <delay.h>
#include <dm.h>
#include <error.h>
#include <sensor.h>
#include <stddef.h>
#include <timer.h>
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

int
enable_temperature_polling(void)
{
	int err;
	struct device *sensor;

	if ((sensor = dm_first_dev_by_class(DM_CLASS_SENSOR)))
		return ENODEV;

	/* Poll temperature periodically. */
	if ((err = timer_run_periodic(sensor_poll_temp, sensor)))
		return err;

	return SUCCESS;
}
