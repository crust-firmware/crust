/*
 * Copyright © 2017-2018 The Crust Firmware Authors.
 * SPDX-License-Identifier: (BSD-3-Clause OR GPL-2.0)
 */

#ifndef DRIVERS_IRQCHIP_H
#define DRIVERS_IRQCHIP_H

#include <debug.h>
#include <dm.h>
#include <stdint.h>

#define IRQCHIP_OPS(dev) ((struct irqchip_driver_ops *)((dev)->drv->ops))

typedef void (*irq_handler)(struct device *);

struct irq_vector {
	struct device *dev;
	irq_handler    handler;
};

struct irqchip_driver_ops {
	int  (*disable)(struct device *irqdev, struct device *dev);
	int  (*enable)(struct device *irqdev, struct device *dev,
	               irq_handler handler);
	void (*irq)(struct device *irqdev);
};

static inline int
irqchip_disable(struct device *dev)
{
	struct device *irqdev = dev->irqdev;

	assert(irqdev);

	return IRQCHIP_OPS(irqdev)->disable(irqdev, dev);
}

static inline int
irqchip_enable(struct device *dev, irq_handler handler)
{
	struct device *irqdev = dev->irqdev;

	assert(irqdev);

	return IRQCHIP_OPS(irqdev)->enable(irqdev, dev, handler);
}

void irqchip_irq(void);

int irqchip_register_device(struct device *dev);

#endif /* DRIVERS_IRQCHIP_H */
