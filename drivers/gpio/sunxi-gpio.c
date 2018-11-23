/*
 * Copyright © 2017-2018 The Crust Firmware Authors.
 * SPDX-License-Identifier: BSD-3-Clause OR GPL-2.0-only
 */

#include <dm.h>
#include <error.h>
#include <gpio.h>
#include <mmio.h>
#include <stdbool.h>
#include <stddef.h>
#include <util.h>
#include <gpio/sunxi-gpio.h>

#define CONFIG_REG(port, index) (0x0000 + 0x24 * (port) + ((index) / 8) * 4)
#define CONFIG_OFFSET(index)    (((pin) % 8) * 4)
#define CONFIG_MASK(index)      BITMASK(CONFIG_OFFSET(index), 3)

#define DATA_REG(port)          (0x0010 + 0x24 * (port))

#define DRIVE_REG(port, index)  (0x0014 + 0x24 * (port) + ((index) / 16) * 2)

#define PULL_REG(port, index)   (0x001c + 0x24 * (port) + ((index) / 16) * 2)

#define PINS_PER_PORT           32
#define PIN_INDEX(pin)          ((pin) % PINS_PER_PORT)
#define PIN_PORT(pin)           ((pin) / PINS_PER_PORT)

/* The most implemented ports on any supported device. */
#define MAX_PORTS               8

static int
sunxi_gpio_get_value(struct device *dev, uint8_t pin, bool *value)
{
	uint8_t index = PIN_INDEX(pin);
	uint8_t port  = PIN_PORT(pin);

	*value = mmio_read32(dev->regs + DATA_REG(port)) & BIT(index);

	return SUCCESS;
}

static int
sunxi_gpio_set_mode(struct device *dev, uint8_t pin, uint8_t mode)
{
	uint8_t index = PIN_INDEX(pin);
	uint8_t port  = PIN_PORT(pin);

	/* Set pin function configuration. */
	mmio_clearsetbits32(dev->regs + CONFIG_REG(port, index),
	                    CONFIG_MASK(index), mode << CONFIG_OFFSET(index));

	return SUCCESS;
}

static int
sunxi_gpio_set_value(struct device *dev, uint8_t pin, bool value)
{
	uint8_t index = PIN_INDEX(pin);
	uint8_t port  = PIN_PORT(pin);

	/* Set the pin to the specified value. */
	mmio_clearsetbits32(dev->regs + DATA_REG(port),
	                    BIT(index), value << index);

	return SUCCESS;
}

static int
sunxi_gpio_probe(struct device *dev)
{
	int err;

	if ((err = dm_setup_clocks(dev, 1)))
		return err;

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
