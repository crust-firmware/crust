/*
 * Copyright © 2017-2018 The Crust Firmware Authors.
 * SPDX-License-Identifier: (BSD-3-Clause OR GPL-2.0)
 */

#include <debug.h>
#include <dm.h>
#include <error.h>
#include <irqchip.h>
#include <mmio.h>
#include <stddef.h>
#include <util.h>
#include <irqchip/sun4i-intc.h>

#define INTC_VECTOR_REG    0x0000
#define INTC_BASE_ADDR_REG 0x0004
#define INTC_PROTECT_REG   0x0008
#define INTC_NMI_CTRL_REG  0x000c
#define INTC_IRQ_PEND_REG  0x0010
#define INTC_EN_REG        0x0040
#define INTC_MASK_REG      0x0050
#define INTC_RESP_REG      0x0060

static inline struct irq_vector *
get_vector(struct device *dev, uint8_t irq)
{
	return &((struct irq_vector *)dev->drvdata)[irq];
}

static int
sun4i_intc_disable(struct device *dev, uint8_t irq)
{
	struct irq_vector *vector = get_vector(dev, irq);

	assert(irq < SUN4I_INTC_IRQS);
	assert(vector->handler != NULL);

	/* Disable the IRQ. */
	mmio_clearbits32(dev->regs + INTC_EN_REG, BIT(irq));

	/* Remove the IRQ vector callback from the vector table. */
	vector->handler = NULL;

	return SUCCESS;
}

static int
sun4i_intc_enable(struct device *dev, uint8_t irq, irq_handler handler,
                  struct device *child)
{
	struct irq_vector *vector = get_vector(dev, irq);

	assert(irq < SUN4I_INTC_IRQS);
	assert(handler != NULL);
	assert(vector->handler == NULL);

	/* Copy the IRQ vector callback to the vector table. */
	vector->dev     = child;
	vector->handler = handler;

	/* Enable the IRQ. */
	mmio_setbits32(dev->regs + INTC_EN_REG, BIT(irq));

	return SUCCESS;
}

static void
sun4i_intc_irq(struct device *dev)
{
	uint8_t irq;

	/* Get current IRQ. */
	while ((irq = mmio_read32(dev->regs + INTC_VECTOR_REG) >> 2)) {
		struct irq_vector *vector = get_vector(dev, irq);

		/* Call the registered callback. */
		assert(vector->handler != NULL);
		vector->handler(vector->dev);

		/* Clear the IRQ pending status. */
		mmio_setbits32(dev->regs + INTC_IRQ_PEND_REG, BIT(irq));
	}
}

static const struct irqchip_driver_ops sun4i_intc_driver_ops = {
	.disable = sun4i_intc_disable,
	.enable  = sun4i_intc_enable,
	.irq     = sun4i_intc_irq,
};

static int
sun4i_intc_probe(struct device *dev)
{
	int err;

	/* Ensure an IRQ vector table was allocated. */
	assert(dev->drvdata);

	/* Clear the table base address (just return IRQ numbers). */
	mmio_write32(dev->regs + INTC_BASE_ADDR_REG, 0);

	/* Disable, unmask, and clear the status of all IRQs. */
	mmio_write32(dev->regs + INTC_EN_REG, 0);
	mmio_write32(dev->regs + INTC_MASK_REG, 0);
	mmio_write32(dev->regs + INTC_IRQ_PEND_REG, ~0);

	/* Register this device with the irqchip framework. */
	if ((err = irqchip_register_device(dev)))
		return err;

	return SUCCESS;
}

const struct driver sun4i_intc_driver = {
	.name  = "sun4i-intc",
	.class = DM_CLASS_IRQCHIP,
	.probe = sun4i_intc_probe,
	.ops   = &sun4i_intc_driver_ops,
};
