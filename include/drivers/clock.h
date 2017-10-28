/*
 * Copyright © 2017 Samuel Holland <samuel@sholland.org>
 * SPDX-License-Identifier: (BSD-3-Clause OR GPL-2.0)
 */

#ifndef DRIVERS_CLOCK_H
#define DRIVERS_CLOCK_H

#include <dm.h>
#include <stdint.h>

#define CLOCK_OPS(ops) ((struct clock_driver_ops *)(ops))

struct clock_driver_ops {
	uint32_t class;
	int      (*disable)(struct device *clockdev, struct device *dev);
	int      (*enable)(struct device *clockdev, struct device *dev);
	int      (*set_freq)(struct device *clockdev, struct device *dev,
	                     uint32_t hz);
};

static inline int
clock_disable(struct device *dev)
{
	struct device *clockdev = dev->clockdev;

	return CLOCK_OPS(clockdev->drv->ops)->disable(clockdev, dev);
}

static inline int
clock_enable(struct device *dev)
{
	struct device *clockdev = dev->clockdev;

	return CLOCK_OPS(clockdev->drv->ops)->enable(clockdev, dev);
}

static inline int
clock_set_freq(struct device *dev, uint32_t hz)
{
	struct device *clockdev = dev->clockdev;

	return CLOCK_OPS(clockdev->drv->ops)->set_freq(clockdev, dev, hz);
}

#endif /* DRIVERS_CLOCK_H */
