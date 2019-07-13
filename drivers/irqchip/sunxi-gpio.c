/*
 * Copyright © 2017-2019 The Crust Firmware Authors.
 * SPDX-License-Identifier: BSD-3-Clause OR GPL-2.0-only
 */

#include <debug.h>
#include <dm.h>
#include <error.h>
#include <intrusive.h>
#include <mmio.h>
#include <stdbool.h>
#include <util.h>
#include <clock/sunxi-ccu.h>
#include <irqchip/sun4i-intc.h>
#include <irqchip/sunxi-gpio.h>
#include <platform/devices.h>

#define INT_CONFIG_REG(port, pin) (0x0200 + 0x20 * (port) + ((pin) / 8) * 4)
#define INT_CONFIG_OFFSET(pin)    (((pin) % 8) * 4)
#define INT_CONFIG_MASK(pin)      GENMASK(INT_CONFIG_OFFSET(pin) + 3, \
	                                  INT_CONFIG_OFFSET(pin))

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
	struct irq_handle **list =
		&container_of(dev, struct irqchip_device, dev)->list;
	uint8_t irq  = handle->irq;
	uint8_t pin  = IRQ_PIN(irq);
	uint8_t port = IRQ_PORT(irq);

	/* Prepend the handle onto the list of IRQs. */
	handle->next = *list;
	*list        = handle;

	/* Set the IRQ mode. */
	mmio_clrset_32(dev->regs + INT_CONFIG_REG(port, pin),
	               INT_CONFIG_MASK(pin),
	               handle->mode << INT_CONFIG_OFFSET(pin));

	/* Enable the IRQ. */
	mmio_set_32(dev->regs + INT_CONTROL_REG(port), BIT(pin));

	return SUCCESS;
}

static bool
sunxi_gpio_irq(struct device *dev)
{
	struct irq_handle **list =
		&container_of(dev, struct irqchip_device, dev)->list;
	uint32_t reg;
	bool handled = false;

	for (size_t port = 0; port < MAX_PORTS; ++port) {
		/* Check the status of the IRQs for this port. */
		if (!(reg = mmio_read_32(dev->regs + INT_STATUS_REG(port))))
			continue;

		/* Call the registered callback for each pending IRQ. */
		for (size_t pin = 0; pin < PINS_PER_PORT; ++pin) {
			if (!(reg & BIT(pin)))
				continue;

			struct irq_handle *handle = *list;
			while (handle != NULL) {
				if (handle->irq == SUNXI_GPIO_IRQ(port, pin) &&
				    handle->fn(handle->dev)) {
					handled = true;
					break;
				}
				handle = handle->next;
			}
		}

		/* Clear the handled pending IRQs for this port. */
		mmio_write_32(dev->regs + INT_STATUS_REG(port), reg);
	}

	return handled;
}

static int
sunxi_gpio_irqchip_probe(struct device *dev)
{
	int err;

	if ((err = dm_setup_clocks(dev, 1)))
		return err;

	/* Disable and clear all IRQs. */
	for (size_t port = 0; port < MAX_PORTS; ++port) {
		mmio_write_32(dev->regs + INT_CONTROL_REG(port), 0);
		mmio_write_32(dev->regs + INT_STATUS_REG(port), ~0);
	}

	return dm_setup_irq(dev, sunxi_gpio_irq);
}

static const struct irqchip_driver sunxi_gpio_irqchip_driver = {
	.drv = {
		.class = DM_CLASS_IRQCHIP,
		.probe = sunxi_gpio_irqchip_probe,
	},
	.ops = {
		.enable = sunxi_gpio_enable,
	},
};

struct irqchip_device r_pio_irqchip = {
	.dev = {
		.name   = "r_pio_irqchip",
		.regs   = DEV_R_PIO,
		.drv    = &sunxi_gpio_irqchip_driver.drv,
		.clocks = CLOCK_PARENT(r_ccu, R_CCU_CLOCK_R_PIO),
		.irq    = IRQ_HANDLE {
			.dev = &r_intc.dev,
			.irq = IRQ_R_PIO_PL,
		},
	},
};
