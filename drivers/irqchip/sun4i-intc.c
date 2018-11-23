/*
 * Copyright © 2017-2018 The Crust Firmware Authors.
 * SPDX-License-Identifier: BSD-3-Clause OR GPL-2.0-only
 */

#include <dm.h>
#include <error.h>
#include <mmio.h>
#include <spr.h>
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

#define HANDLE_LIST(dev)   ((struct irq_handle *)(dev)->drvdata)

static int
sun4i_intc_enable(struct device *dev, struct irq_handle *handle)
{
	/* Prepend the handle onto the list of IRQs. */
	handle->next = HANDLE_LIST(dev);
	dev->drvdata = (uintptr_t)handle;

	/* Enable the IRQ. */
	mmio_setbits32(dev->regs + INTC_EN_REG, BIT(handle->irq));

	return SUCCESS;
}

void
sun4i_intc_irq(struct device *dev)
{
	struct irq_handle *handle = HANDLE_LIST(dev);
	/* Get the number of the current IRQ. */
	uint8_t irq = mmio_read32(dev->regs + INTC_VECTOR_REG) >> 2;

	/* Call the registered callback. */
	while (handle != NULL) {
		if (handle->irq == irq) {
			handle->fn(handle->dev);
			break;
		}
		handle = handle->next;
	}

	/* Clear the IRQ pending status. */
	mmio_setbits32(dev->regs + INTC_IRQ_PEND_REG, BIT(irq));
}

static int
sun4i_intc_probe(struct device *dev)
{
	/* Clear the table base address (just return IRQ numbers). */
	mmio_write32(dev->regs + INTC_BASE_ADDR_REG, 0);

	/* Disable, unmask, and clear the status of all IRQs. */
	mmio_write32(dev->regs + INTC_EN_REG, 0);
	mmio_write32(dev->regs + INTC_MASK_REG, 0);
	mmio_write32(dev->regs + INTC_IRQ_PEND_REG, ~0);

	/* Enable the CPU external interrupt input. */
	mtspr(SPR_SYS_SR_ADDR, SPR_SYS_SR_IEE_SET(mfspr(SPR_SYS_SR_ADDR), 1));

	return SUCCESS;
}

const struct irqchip_driver sun4i_intc_driver = {
	.drv = {
		.class = DM_CLASS_IRQCHIP,
		.probe = sun4i_intc_probe,
	},
	.ops = {
		.enable = sun4i_intc_enable,
	},
};
