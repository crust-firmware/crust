/*
 * Copyright © 2017-2018 The Crust Firmware Authors.
 * SPDX-License-Identifier: BSD-3-Clause OR GPL-2.0-only
 */

#ifndef DRIVERS_WATCHDOG_H
#define DRIVERS_WATCHDOG_H

#include <dm.h>
#include <intrusive.h>
#include <stdint.h>

#define WATCHDOG_OPS(dev) \
	(&container_of((dev)->drv, struct watchdog_driver, drv)->ops)

struct watchdog_driver_ops {
	int (*disable)(struct device *dev);
	int (*enable)(struct device *dev, uint32_t timeout);
};

struct watchdog_driver {
	const struct driver              drv;
	const struct watchdog_driver_ops ops;
};

/**
 * Disable the watchdog.
 *
 * @param dev The watchdog device.
 */
static inline int
watchdog_disable(struct device *dev)
{
	return WATCHDOG_OPS(dev)->disable(dev);
}

/**
 * Enable and restart the watchdog.
 *
 * @param dev     The watchdog device.
 * @param timeout The watchdog timeout in clock cycles.
 */
static inline int
watchdog_enable(struct device *dev, uint32_t timeout)
{
	return WATCHDOG_OPS(dev)->enable(dev, timeout);
}

#endif /* DRIVERS_WATCHDOG_H */
