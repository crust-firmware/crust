/*
 * Copyright © 2017-2019 The Crust Firmware Authors.
 * SPDX-License-Identifier: BSD-3-Clause OR GPL-2.0-only
 */

#include <dm.h>
#include <error.h>
#include <intrusive.h>
#include <limits.h>
#include <mmio.h>
#include <util.h>
#include <irq/sun4i-intc.h>
#include <platform/devices.h>

#define INTC_VECTOR_REG    0x0000
#define INTC_BASE_ADDR_REG 0x0004
#define INTC_PROTECT_REG   0x0008
#define INTC_NMI_CTRL_REG  0x000c
#define INTC_IRQ_PEND_REG  0x0010
#define INTC_EN_REG        0x0040
#define INTC_MASK_REG      0x0050
#define INTC_RESP_REG      0x0060

static int
sun4i_intc_enable(struct device *dev, struct irq_handle *handle)
{
	struct sun4i_intc *this =
		container_of(dev, struct sun4i_intc, dev);

	/* Prepend the handle onto the list of IRQs. */
	handle->next = this->list;
	this->list   = handle;

	/* Enable the IRQ. */
	mmio_set_32(dev->regs + INTC_EN_REG, BIT(handle->irq));

	return SUCCESS;
}

static void
sun4i_intc_poll(struct device *dev)
{
	struct sun4i_intc *this =
		container_of(dev, struct sun4i_intc, dev);
	uint32_t status = mmio_read_32(dev->regs + INTC_EN_REG) &
	                  mmio_read_32(dev->regs + INTC_IRQ_PEND_REG);

	for (int i = 0; i < WORD_BIT; ++i) {
		if (status & BIT(i)) {
			const struct irq_handle *handle = this->list;
			/* Call the registered callback. */
			while (handle != NULL) {
				if (handle->irq == i &&
				    handle->handler(handle))
					break;
				handle = handle->next;
			}
			/* Clear the IRQ pending status if handled. */
			if (handle != NULL) {
				mmio_write_32(dev->regs + INTC_IRQ_PEND_REG,
				              BIT(i));
			}
		}
	}
}

static int
sun4i_intc_probe(struct device *dev)
{
	/* Clear the table base address (just return IRQ numbers). */
	mmio_write_32(dev->regs + INTC_BASE_ADDR_REG, 0);

	/* Disable, unmask, and clear the status of all IRQs. */
	mmio_write_32(dev->regs + INTC_EN_REG, 0);
	mmio_write_32(dev->regs + INTC_MASK_REG, 0);
	mmio_write_32(dev->regs + INTC_IRQ_PEND_REG, ~0);

	return SUCCESS;
}

static const struct irq_driver sun4i_intc_driver = {
	.drv = {
		.probe = sun4i_intc_probe,
		.poll  = sun4i_intc_poll,
	},
	.ops = {
		.enable = sun4i_intc_enable,
	},
};

struct sun4i_intc r_intc = {
	.dev = {
		.name = "r_intc",
		.regs = DEV_R_INTC,
		.drv  = &sun4i_intc_driver.drv,
	},
};
