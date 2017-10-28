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

#define INTC_VECTOR_REG    0x0000
#define INTC_BASE_ADDR_REG 0x0004
#define INTC_PROTECT_REG   0x0008
#define INTC_NMI_CTRL_REG  0x000c
#define INTC_IRQ_PEND_REG  0x0010
#define INTC_EN_REG        0x0040
#define INTC_MASK_REG      0x0050
#define INTC_RESP_REG      0x0060

#define INTC_CHANNELS      32

static struct {
	struct device *dev;
	void           (*handler)(struct device *);
} irq_vectors[INTC_CHANNELS];

static int
sun4i_intc_irq(struct device *irqdev)
{
	uint32_t irq;

	/* Get current IRQ. */
	while ((irq = mmio_read32(irqdev->address + INTC_VECTOR_REG) >> 2)) {
		/* Call registered handler. */
		if (likely(irq_vectors[irq].handler)) {
			irq_vectors[irq].handler(irq_vectors[irq].dev);
		} else {
			warn("no ISR registered for IRQ %d", irq);
			if (irq_vectors[irq].dev) {
				debug("IRQ %d last registered to device %s",
				      irq,
				      irq_vectors[irq].dev->name);
			}
		}

		/* Clear IRQ pending status. */
		mmio_setbits32(irqdev->address + INTC_IRQ_PEND_REG, BIT(irq));
	}

	return SUCCESS;
}

static int
sun4i_intc_probe(struct device *dev)
{
	int err;

	/* Clear base address (just return IRQ numbers). */
	mmio_write32(dev->address + INTC_BASE_ADDR_REG, 0);

	/* Disable, unmask, and clear status for all IRQs. */
	mmio_write32(dev->address + INTC_EN_REG, 0);
	mmio_write32(dev->address + INTC_MASK_REG, 0);
	mmio_write32(dev->address + INTC_IRQ_PEND_REG, ~0);

	/* Register this device with the irqchip framework. */
	if ((err = irqchip_device_register(dev)))
		return err;

	return SUCCESS;
}

static int
sun4i_intc_register_irq(struct device *irqdev, struct device *dev,
                        irq_handler handler)
{
	uint8_t irq = dev->irq;

	assert(handler);
	assert(!irq_vectors[irq].handler);

	/* Add IRQ vector. */
	irq_vectors[irq].dev     = dev;
	irq_vectors[irq].handler = handler;

	debug("IRQ %d now registered to device %s", irq, dev->name);

	/* Enable IRQ. */
	mmio_setbits32(irqdev->address + INTC_EN_REG, BIT(irq));

	debug("IRQ %d now enabled for device %s", irq, dev->name);

	return SUCCESS;
}

static int
sun4i_intc_unregister_irq(struct device *irqdev, struct device *dev)
{
	uint8_t irq = dev->irq;

	assert(irq_vectors[irq].dev == dev);
	assert(irq_vectors[irq].handler);

	/* Disable IRQ. */
	mmio_clearbits32(irqdev->address + INTC_EN_REG, BIT(irq));

	/* Remove IRQ vector (but remember the last device). */
	irq_vectors[irq].handler = NULL;

	return SUCCESS;
}

static struct irqchip_driver_ops sun4i_intc_driver_ops = {
	.class          = DM_CLASS_IRQCHIP,
	.irq            = sun4i_intc_irq,
	.register_irq   = sun4i_intc_register_irq,
	.unregister_irq = sun4i_intc_unregister_irq,
};

struct driver sun4i_intc_driver = {
	.name  = "sun4i-intc",
	.probe = sun4i_intc_probe,
	.ops   = &sun4i_intc_driver_ops,
};
