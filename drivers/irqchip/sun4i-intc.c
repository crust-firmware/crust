/*
 * Copyright © 2017 Samuel Holland <samuel@sholland.org>
 * SPDX-License-Identifier: (BSD-3-Clause OR GPL-2.0)
 */

#include <debug.h>
#include <dm.h>
#include <error.h>
#include <mmio.h>
#include <stddef.h>
#include <util.h>
#include <drivers/irqchip.h>
#include <drivers/irqchip/sun4i-intc.h>

#define INTC_VECTOR_REG    0x0000
#define INTC_BASE_ADDR_REG 0x0004
#define INTC_PROTECT_REG   0x0008
#define INTC_NMI_CTRL_REG  0x000c
#define INTC_IRQ_PEND_REG  0x0010
#define INTC_EN_REG        0x0040
#define INTC_MASK_REG      0x0050
#define INTC_RESP_REG      0x0060

static struct irq_vector *
get_vector(struct device *irqdev, uintptr_t irq)
{
	return &((struct irq_vector *)irqdev->drvdata)[irq];
}

static int
sun4i_intc_irq(struct device *irqdev)
{
	uintptr_t irq;

	/* Get current IRQ. */
	while ((irq = mmio_read32(irqdev->regs + INTC_VECTOR_REG) >> 2)) {
		struct irq_vector *vector = get_vector(irqdev, irq);

		/* Call registered handler. */
		if (likely(vector->handler)) {
			vector->handler(vector->dev);
		} else {
			warn("No handler registered for %s IRQ %d",
			     irqdev->name, irq);
			if (vector->dev) {
				debug("IRQ %d last registered to device %s",
				      irq,
				      vector->dev->name);
			}
		}

		/* Clear IRQ pending status. */
		mmio_setbits32(irqdev->regs + INTC_IRQ_PEND_REG, BIT(irq));
	}

	return SUCCESS;
}

static int
sun4i_intc_probe(struct device *dev)
{
	int err;

	assert(dev->drvdata);

	/* Clear base address (just return IRQ numbers). */
	mmio_write32(dev->regs + INTC_BASE_ADDR_REG, 0);

	/* Disable, unmask, and clear status for all IRQs. */
	mmio_write32(dev->regs + INTC_EN_REG, 0);
	mmio_write32(dev->regs + INTC_MASK_REG, 0);
	mmio_write32(dev->regs + INTC_IRQ_PEND_REG, ~0);

	/* Register this device with the irqchip framework. */
	if ((err = irqchip_device_register(dev)))
		return err;

	return SUCCESS;
}

static int
sun4i_intc_register_irq(struct device *irqdev, struct device *dev,
                        irq_handler handler)
{
	uintptr_t irq             = dev->irq;
	struct irq_vector *vector = get_vector(irqdev, irq);

	assert(irq < SUN4I_INTC_IRQS);
	assert(handler);
	assert(!vector->handler);

	/* Add IRQ vector. */
	vector->dev     = dev;
	vector->handler = handler;

	debug("IRQ %d now registered to device %s", irq, dev->name);

	/* Enable IRQ. */
	mmio_setbits32(irqdev->regs + INTC_EN_REG, BIT(irq));

	debug("IRQ %d now enabled for device %s", irq, dev->name);

	return SUCCESS;
}

static int
sun4i_intc_unregister_irq(struct device *irqdev, struct device *dev)
{
	uintptr_t irq             = dev->irq;
	struct irq_vector *vector = get_vector(irqdev, irq);

	assert(irq < SUN4I_INTC_IRQS);
	assert(vector->dev == dev);
	assert(vector->handler);

	/* Disable IRQ. */
	mmio_clearbits32(irqdev->regs + INTC_EN_REG, BIT(irq));

	/* Remove IRQ vector (but remember the last device). */
	vector->handler = NULL;

	return SUCCESS;
}

static const struct irqchip_driver_ops sun4i_intc_driver_ops = {
	.irq            = sun4i_intc_irq,
	.register_irq   = sun4i_intc_register_irq,
	.unregister_irq = sun4i_intc_unregister_irq,
};

const struct driver sun4i_intc_driver = {
	.name  = "sun4i-intc",
	.class = DM_CLASS_IRQCHIP,
	.probe = sun4i_intc_probe,
	.ops   = &sun4i_intc_driver_ops,
};
