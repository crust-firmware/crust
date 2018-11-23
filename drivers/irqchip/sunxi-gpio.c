/*
 * Copyright © 2017-2018 The Crust Firmware Authors.
 * SPDX-License-Identifier: BSD-3-Clause OR GPL-2.0-only
 */

#include <debug.h>
#include <dm.h>
#include <error.h>
#include <mmio.h>
#include <util.h>
#include <irqchip/sunxi-gpio.h>

#define HANDLE_LIST(dev)          ((struct irq_handle *)(dev)->drvdata)

#define INT_CONFIG_REG(port, pin) (0x0200 + 0x20 * (port) + ((pin) / 8) * 4)
#define INT_CONFIG_OFFSET(pin)    (((pin) % 8) * 4)
#define INT_CONFIG_MASK(pin)      BITMASK(INT_CONFIG_OFFSET(pin), 4)

#define INT_CONTROL_REG(port)     (0x0210 + 0x20 * (port))

#define INT_STATUS_REG(port)      (0x0214 + 0x20 * (port))

#define PINS_PER_PORT             32
#define IRQ_PIN(irq)              ((irq) % PINS_PER_PORT)
#define IRQ_PORT(irq)             ((irq) / PINS_PER_PORT)

/* The most interrupt-enabled ports on any supported device. */
#define MAX_PORTS                 1

static int
sunxi_gpio_enable(struct device *dev, struct irq_handle *handle)
{
	uint8_t irq  = handle->irq;
	uint8_t pin  = IRQ_PIN(irq);
	uint8_t port = IRQ_PORT(irq);

	/* Prepend the handle onto the list of IRQs. */
	handle->next = HANDLE_LIST(dev);
	dev->drvdata = (uintptr_t)handle;

	/* Set the IRQ mode. */
	mmio_clearsetbits32(dev->regs + INT_CONFIG_REG(port, pin),
	                    INT_CONFIG_MASK(pin),
	                    handle->mode << INT_CONFIG_OFFSET(pin));

	/* Enable the IRQ. */
	mmio_setbits32(dev->regs + INT_CONTROL_REG(port), BIT(pin));

	return SUCCESS;
}

static void
sunxi_gpio_irq(void *param)
{
	struct device *dev = param;
	uint32_t reg;

	for (size_t port = 0; port < MAX_PORTS; ++port) {
		/* Check the status of the IRQs for this port. */
		if ((reg = mmio_read32(dev->regs + INT_STATUS_REG(port))) == 0)
			continue;

		/* Call the registered callback for each pending IRQ. */
		for (size_t pin = 0; pin < PINS_PER_PORT; ++pin) {
			if (!(reg & BIT(pin)))
				continue;

			struct irq_handle *handle = HANDLE_LIST(dev);
			while (handle != NULL) {
				if (handle->irq == SUNXI_GPIO_IRQ(port, pin)) {
					handle->fn(handle->dev);
					break;
				}
				handle = handle->next;
			}
		}

		/* Clear the handled pending IRQs for this port. */
		mmio_write32(dev->regs + INT_STATUS_REG(port), reg);
	}
}

static int
sunxi_gpio_irqchip_probe(struct device *dev)
{
	int err;

	if ((err = dm_setup_clocks(dev, 1)))
		return err;

	/* Disable and clear all IRQs. */
	for (size_t port = 0; port < MAX_PORTS; ++port) {
		mmio_write32(dev->regs + INT_CONTROL_REG(port), 0);
		mmio_write32(dev->regs + INT_STATUS_REG(port), ~0);
	}

	return dm_setup_irq(dev, sunxi_gpio_irq);
}

const struct irqchip_driver sunxi_gpio_irqchip_driver = {
	.drv = {
		.class = DM_CLASS_IRQCHIP,
		.probe = sunxi_gpio_irqchip_probe,
	},
	.ops = {
		.enable = sunxi_gpio_enable,
	},
};
