/*
 * Copyright © 2017-2019 The Crust Firmware Authors.
 * SPDX-License-Identifier: BSD-3-Clause OR GPL-2.0-only
 */

#include <dm.h>
#include <error.h>
#include <irq.h>

#define IRQ_OPS(dev) \
	(&container_of((dev)->drv, struct irq_driver, drv)->ops)

int
irq_get(struct irq_handle *irq)
{
	struct device *dev = irq->dev;
	int err;

	/* Ensure the controller's driver is loaded. */
	device_probe(dev);

	/* Enable the IRQ line. */
	if ((err = IRQ_OPS(dev)->enable(dev, irq)))
		return err;

	return SUCCESS;
}
