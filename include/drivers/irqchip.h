/*
 * Copyright © 2017 Samuel Holland <samuel@sholland.org>
 * SPDX-License-Identifier: (BSD-3-Clause OR GPL-2.0)
 */

#ifndef DRIVERS_IRQCHIP_H
#define DRIVERS_IRQCHIP_H

#include <dm.h>
#include <stdint.h>

#define IRQCHIP_OPS(ops) ((struct irqchip_driver_ops *)(ops))

typedef void (*irq_handler)(struct device *);

struct irqchip_driver_ops {
	uint32_t class;
	int      (*irq)(struct device *irqdev);
	int      (*register_irq)(struct device *irqdev, struct device *dev,
	                         irq_handler handler);
	int      (*unregister_irq)(struct device *irqdev, struct device *dev);
};

int irqchip_device_register(struct device *dev);
int irqchip_irq(void);

static inline int
irqchip_register_irq(struct device *dev, irq_handler handler)
{
	struct device *irqdev = dev->irqdev;

	return IRQCHIP_OPS(irqdev->drv->ops)->register_irq(irqdev, dev,
	                                                   handler);
}

static inline int
irqchip_unregister_irq(struct device *dev)
{
	struct device *irqdev = dev->irqdev;

	return IRQCHIP_OPS(irqdev->drv->ops)->unregister_irq(irqdev, dev);
}

#endif /* DRIVERS_IRQCHIP_H */
