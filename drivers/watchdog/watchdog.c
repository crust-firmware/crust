/*
 * Copyright © 2017-2020 The Crust Firmware Authors.
 * SPDX-License-Identifier: BSD-3-Clause OR GPL-2.0-only
 */

#include <device.h>
#include <intrusive.h>
#include <stdint.h>
#include <watchdog.h>

#include "watchdog.h"

/**
 * Get the ops for the watchdog controller device.
 */
static inline const struct watchdog_driver_ops *
watchdog_ops_for(const struct device *dev)
{
	const struct watchdog_driver *drv =
		container_of(dev->drv, const struct watchdog_driver, drv);

	return &drv->ops;
}

void
watchdog_disable(const struct device *dev)
{
	watchdog_ops_for(dev)->disable(dev);
}

int
watchdog_enable(const struct device *dev, uint32_t timeout)
{
	return watchdog_ops_for(dev)->enable(dev, timeout);
}

void
watchdog_restart(const struct device *dev)
{
	watchdog_ops_for(dev)->restart(dev);
}
