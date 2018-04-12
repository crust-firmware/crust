/*
 * Copyright © 2017-2018 The Crust Firmware Authors.
 * SPDX-License-Identifier: (BSD-3-Clause OR GPL-2.0)
 */

#include <clock.h>
#include <dm.h>
#include <error.h>
#include <gpio.h>
#include <irqchip.h>
#include <mmio.h>
#include <stdbool.h>
#include <stddef.h>
#include <util.h>
#include <gpio/sunxi-gpio.h>

#define MAX_PORTS      8

#define PIN_INDEX(pin) ((pin) & BITMASK(0, 5))
#define PIN_PORT(pin)  ((pin) >> 5)

#define PIN_MODE(mode) ((mode) & BITMASK(0, 3))

#define CONFIG_REG(pin) \
	(0x0000 + PIN_PORT(pin) * 0x24 + (PIN_INDEX(pin) >> 3) * 4)
#define DATA_REG(pin) \
	(0x0010 + PIN_PORT(pin) * 0x24)

#define CONFIG_OFFSET(pin)    ((PIN_INDEX(pin) & BITMASK(0, 3)) * 4)

#define INT_CONTROL_REG(port) (0x0210 + (port) * 0x20)
#define INT_STATUS_REG(port)  (0x0214 + (port) * 0x20)

static int
sunxi_gpio_get_value(struct device *dev, uint8_t pin, bool *value)
{
	*value = mmio_read32(dev->regs + DATA_REG(pin)) & BIT(PIN_INDEX(pin));

	return SUCCESS;
}

static int
sunxi_gpio_set_mode(struct device *dev, uint8_t pin, uint8_t mode)
{
	/* Verify port exists. */
	if (!(dev->drvdata & BIT(PIN_PORT(pin))))
		return ENODEV;

	mmio_clearsetbits32(dev->regs + CONFIG_REG(pin),
	                    BITMASK(CONFIG_OFFSET(pin), 3),
	                    PIN_MODE(mode) << CONFIG_OFFSET(pin));

	return SUCCESS;
}

static int
sunxi_gpio_set_value(struct device *dev, uint8_t pin, bool value)
{
	/* Set pin to specified val. */
	mmio_clearsetbits32(dev->regs + DATA_REG(pin),
	                    BIT(PIN_INDEX(pin)),
	                    value << PIN_INDEX(pin));

	return SUCCESS;
}

static int
sunxi_gpio_probe(struct device *dev)
{
	int err;

	if ((err = dm_setup_clocks(dev, 1)))
		return err;

	/* Disable and clear all IRQs. */
	for (size_t port = 0; port < MAX_PORTS; ++port) {
		if (dev->drvdata & BIT(port)) {
			mmio_write32(dev->regs + INT_CONTROL_REG(port), 0x0);
			mmio_write32(dev->regs + INT_STATUS_REG(port), ~0);
		}
	}

	return SUCCESS;
}

const struct gpio_driver sunxi_gpio_driver = {
	.drv = {
		.class = DM_CLASS_GPIO,
		.probe = sunxi_gpio_probe,
	},
	.ops = {
		.get_value = sunxi_gpio_get_value,
		.set_mode  = sunxi_gpio_set_mode,
		.set_value = sunxi_gpio_set_value,
	},
};
