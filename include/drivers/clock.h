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
	int (*set_freq)(struct device *clockdev, struct device *dev,
	                uint32_t hz);
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
clock_set_freq(struct device *dev, uint32_t hz)
{
	struct device *clockdev = dev->clockdev;

	return CLOCK_OPS(clockdev)->set_freq(clockdev, dev, hz);
}

#endif /* DRIVERS_CLOCK_H */
