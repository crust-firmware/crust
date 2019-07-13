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

struct irq_handle {
	struct irq_handle *next;
	struct device     *dev;
	const uint8_t      irq;
	const uint8_t      mode;
	bool               (*handler)(const struct irq_handle *);
};

struct irq_driver_ops {
	int (*enable)(struct device *dev, struct irq_handle *handle);
};

struct irq_driver {
	const struct driver         drv;
	const struct irq_driver_ops ops;
};

/**
 * Get a reference to an IRQ line's controller device; then enable the IRQ.
 *
 * @param irq  A handle for the IRQ line
 */
int irq_get(struct irq_handle *irq);

#endif /* DRIVERS_IRQ_H */
