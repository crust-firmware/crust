/*
 * Copyright © 2017-2018 The Crust Firmware Authors.
 * SPDX-License-Identifier: (BSD-3-Clause OR GPL-2.0)
 */

#include <compiler.h>
#include <debug.h>
#include <delay.h>
#include <dm.h>
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
