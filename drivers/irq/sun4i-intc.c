/*
 * Copyright © 2017-2019 The Crust Firmware Authors.
 * SPDX-License-Identifier: BSD-3-Clause OR GPL-2.0-only
 */

#include <dm.h>
#include <error.h>
#include <intrusive.h>
#include <limits.h>
#include <mmio.h>
#include <system_power.h>
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

static inline struct sun4i_intc *
to_sun4i_intc(struct device *dev)
{
	return container_of(dev, struct sun4i_intc, dev);
}

static int
sun4i_intc_enable(struct device *dev, struct irq_handle *handle)
{
	struct sun4i_intc *self = to_sun4i_intc(dev);

	/* Prepend the handle onto the list of IRQs. */
	handle->next = self->list;
	self->list   = handle;

	/* Enable the IRQ. */
	mmio_set_32(self->regs + INTC_EN_REG, BIT(handle->irq));

	return SUCCESS;
}

static void
sun4i_intc_poll(struct device *dev)
{
	struct sun4i_intc *self = to_sun4i_intc(dev);

	uint32_t status = mmio_read_32(self->regs + INTC_EN_REG) &
	                  mmio_read_32(self->regs + INTC_IRQ_PEND_REG);

	for (int i = 0; i < WORD_BIT; ++i) {
		if (status & BIT(i)) {
			const struct irq_handle *handle = self->list;
			/* Call the registered callback. */
			while (handle != NULL) {
				if (handle->irq == i &&
				    handle->handler(handle))
					break;
				handle = handle->next;
			}
			if (handle != NULL) {
				/* Clear the IRQ pending status if handled. */
				mmio_write_32(self->regs + INTC_IRQ_PEND_REG,
				              BIT(i));
			} else {
				/* Wake the system on an unhandled IRQ. */
				system_wakeup();
			}
		}
	}
}

static int
sun4i_intc_probe(struct device *dev)
{
	struct sun4i_intc *self = to_sun4i_intc(dev);

	/* Clear the table base address (just return IRQ numbers). */
	mmio_write_32(self->regs + INTC_BASE_ADDR_REG, 0);

	/* Disable, unmask, and clear the status of all IRQs. */
	mmio_write_32(self->regs + INTC_EN_REG, 0);
	mmio_write_32(self->regs + INTC_MASK_REG, 0);
	mmio_write_32(self->regs + INTC_IRQ_PEND_REG, ~0);

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
		.drv  = &sun4i_intc_driver.drv,
	},
	.regs = DEV_R_INTC,
};
