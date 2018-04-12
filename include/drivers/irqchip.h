/*
 * Copyright © 2017-2018 The Crust Firmware Authors.
 * SPDX-License-Identifier: (BSD-3-Clause OR GPL-2.0)
 */

#ifndef DRIVERS_IRQCHIP_H
#define DRIVERS_IRQCHIP_H

#include <dm.h>
#include <intrusive.h>
#include <stdint.h>
#include <work.h>

#define IRQCHIP_OPS(dev) \
	(&container_of((dev)->drv, struct irqchip_driver, drv)->ops)

struct irqchip_driver_ops {
	int  (*disable)(struct device *dev, uint8_t irq);
	int  (*enable)(struct device *dev, uint8_t irq, callback_t *fn,
	               void *param);
	void (*irq)(struct device *irqdev);
};

struct irqchip_driver {
	const struct driver             drv;
	const struct irqchip_driver_ops ops;
};

static inline int
irqchip_disable(struct device *dev, uint8_t irq)
{
	return IRQCHIP_OPS(dev)->disable(dev, irq);
}

static inline int
irqchip_enable(struct device *dev, uint8_t irq, callback_t *fn, void *param)
{
	return IRQCHIP_OPS(dev)->enable(dev, irq, fn, param);
}

#endif /* DRIVERS_IRQCHIP_H */
