/*
 * Copyright © 2017-2018 The Crust Firmware Authors.
 * SPDX-License-Identifier: (BSD-3-Clause OR GPL-2.0)
 */

#ifndef DRIVERS_IRQCHIP_H
#define DRIVERS_IRQCHIP_H

#include <dm.h>
#include <stdint.h>

#define IRQCHIP_OPS(dev) ((struct irqchip_driver_ops *)((dev)->drv->ops))

typedef void (*irq_handler)(struct device *);

struct irq_vector {
	struct device *dev;
	irq_handler    handler;
};

struct irqchip_driver_ops {
	int (*irq)(struct device *irqdev);
	int (*register_irq)(struct device *irqdev, struct device *dev,
	                    irq_handler handler);
	int (*unregister_irq)(struct device *irqdev, struct device *dev);
};

int irqchip_device_register(struct device *dev);
int irqchip_irq(void);

static inline int
irqchip_register_irq(struct device *dev, irq_handler handler)
{
	struct device *irqdev = dev->irqdev;

	return IRQCHIP_OPS(irqdev)->register_irq(irqdev, dev, handler);
}

static inline int
irqchip_unregister_irq(struct device *dev)
{
	struct device *irqdev = dev->irqdev;

	return IRQCHIP_OPS(irqdev)->unregister_irq(irqdev, dev);
}

#endif /* DRIVERS_IRQCHIP_H */
