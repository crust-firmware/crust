/*
 * Copyright © 2017-2019 The Crust Firmware Authors.
 * SPDX-License-Identifier: BSD-3-Clause OR GPL-2.0-only
 */

#ifndef DRIVERS_IRQ_H
#define DRIVERS_IRQ_H

#include <dm.h>
#include <intrusive.h>
#include <stdbool.h>
#include <stdint.h>

#define IRQ_OPS(dev) \
	(&container_of((dev)->drv, struct irq_driver, drv)->ops)

#define IRQ_HANDLE &(struct irq_handle)

struct irq_handle {
	struct irq_handle *next;
	struct device     *dev;
	bool               (*fn)(struct device *);

	const uint8_t      irq;
	const uint8_t      mode;
};

struct irq_device {
	struct device      dev;
	struct irq_handle *list;
};

struct irq_driver_ops {
	int (*enable)(struct device *dev, struct irq_handle *handle);
};

struct irq_driver {
	const struct driver         drv;
	const struct irq_driver_ops ops;
};

static inline int
irq_enable(struct device *dev, struct irq_handle *handle)
{
	return IRQ_OPS(dev)->enable(dev, handle);
}

#endif /* DRIVERS_IRQ_H */
