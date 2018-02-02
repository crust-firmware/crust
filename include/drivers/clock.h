/*
 * Copyright © 2017-2018 The Crust Firmware Authors.
 * SPDX-License-Identifier: (BSD-3-Clause OR GPL-2.0)
 */

#ifndef DRIVERS_CLOCK_H
#define DRIVERS_CLOCK_H

#include <dm.h>
#include <stdint.h>

#define CLOCK_OPS(dev) ((struct clock_driver_ops *)((dev)->drv->ops))

struct clock_driver_ops {
	int (*disable)(struct device *clockdev, struct device *dev);
	int (*enable)(struct device *clockdev, struct device *dev);
	int (*get_rate)(struct device *clockdev, struct device *dev,
	                uint32_t *rate);
	int (*set_rate)(struct device *clockdev, struct device *dev,
	                uint32_t rate);
};

static inline int
clock_disable(struct device *dev)
{
	struct device *clockdev = dev->clockdev;

	return CLOCK_OPS(clockdev)->disable(clockdev, dev);
}

static inline int
clock_enable(struct device *dev)
{
	struct device *clockdev = dev->clockdev;

	return CLOCK_OPS(clockdev)->enable(clockdev, dev);
}

static inline int
clock_get_rate(struct device *dev, uint32_t *rate)
{
	struct device *clockdev = dev->clockdev;

	return CLOCK_OPS(clockdev)->get_rate(clockdev, dev, rate);
}

static inline int
clock_set_rate(struct device *dev, uint32_t rate)
{
	struct device *clockdev = dev->clockdev;

	return CLOCK_OPS(clockdev)->set_rate(clockdev, dev, rate);
}

#endif /* DRIVERS_CLOCK_H */
