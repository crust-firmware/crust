/*
 * Copyright © 2017-2019 The Crust Firmware Authors.
 * SPDX-License-Identifier: BSD-3-Clause OR GPL-2.0-only
 */

#include <device.h>
#include <error.h>
#include <intrusive.h>
#include <mmio.h>
#include <system_power.h>
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

static inline const struct sun4i_intc *
to_sun4i_intc(const struct device *dev)
{
	return container_of(dev, struct sun4i_intc, dev);
}

static void
sun4i_intc_poll(const struct device *dev)
{
	const struct sun4i_intc *self = to_sun4i_intc(dev);

	uint32_t status = mmio_read_32(self->regs + INTC_EN_REG) &
	                  mmio_read_32(self->regs + INTC_IRQ_PEND_REG);

	/* Wake the system on an incoming IRQ. */
	if (status)
		system_wakeup();
}

static int
sun4i_intc_probe(const struct device *dev)
{
	const struct sun4i_intc *self = to_sun4i_intc(dev);

	/* Clear the table base address (just return IRQ numbers). */
	mmio_write_32(self->regs + INTC_BASE_ADDR_REG, 0);

	/* Disable, unmask, and clear the status of all IRQs. */
	mmio_write_32(self->regs + INTC_EN_REG, 0);
	mmio_write_32(self->regs + INTC_MASK_REG, 0);
	mmio_write_32(self->regs + INTC_IRQ_PEND_REG, ~0);

	return SUCCESS;
}

static const struct driver sun4i_intc_driver = {
	.probe = sun4i_intc_probe,
	.poll  = sun4i_intc_poll,
};

const struct sun4i_intc r_intc = {
	.dev = {
		.name  = "r_intc",
		.drv   = &sun4i_intc_driver,
		.state = DEVICE_STATE_INIT,
	},
	.regs = DEV_R_INTC,
};
