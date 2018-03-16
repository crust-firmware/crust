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
	int (*disable)(struct device *dev, uint8_t id);
	int (*enable)(struct device *dev, uint8_t id);
	int (*get_rate)(struct device *dev, uint8_t id, uint32_t *rate);
	int (*set_rate)(struct device *dev, uint8_t id, uint32_t rate);
};

static inline int
clock_disable(struct device *dev, uint8_t id)
{
	return CLOCK_OPS(dev)->disable(dev, id);
}

static inline int
clock_enable(struct device *dev, uint8_t id)
{
	return CLOCK_OPS(dev)->enable(dev, id);
}

static inline int
clock_get_rate(struct device *dev, uint8_t id, uint32_t *rate)
{
	return CLOCK_OPS(dev)->get_rate(dev, id, rate);
}

static inline int
clock_set_rate(struct device *dev, uint8_t id, uint32_t rate)
{
	return CLOCK_OPS(dev)->set_rate(dev, id, rate);
}

#endif /* DRIVERS_CLOCK_H */
