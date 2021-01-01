/*
 * Copyright © 2017-2021 The Crust Firmware Authors.
 * SPDX-License-Identifier: BSD-3-Clause OR GPL-2.0-only
 */

#include <bitfield.h>
#include <error.h>
#include <limits.h>
#include <mmio.h>
#include <stdbool.h>
#include <stdint.h>
#include <clock/ccu.h>
#include <gpio/sunxi-gpio.h>
#include <platform/devices.h>

#include "gpio.h"

#define MODE_WIDTH           4
#define MODE_PPW             (WORD_BIT / MODE_WIDTH)
#define MODE_REG(port, pin)  (0x0000 + 0x24 * (port) + 4 * ((pin) / MODE_PPW))
#define MODE_BIT(pin)        (MODE_WIDTH * ((pin) % MODE_PPW))

#define DATA_WIDTH           1
#define DATA_PPW             (WORD_BIT / DATA_WIDTH)
#define DATA_REG(port)       (0x0010 + 0x24 * (port))
#define DATA_BIT(pin)        (pin)

#define DRIVE_WIDTH          2
#define DRIVE_PPW            (WORD_BIT / DRIVE_WIDTH)
#define DRIVE_REG(port, pin) (0x0014 + 0x24 * (port) + 4 * ((pin) / DRIVE_PPW))
#define DRIVE_BIT(pin)       (DRIVE_WIDTH * ((pin) % DRIVE_PPW))

#define PULL_WIDTH           2
#define PULL_PPW             (WORD_BIT / PULL_WIDTH)
#define PULL_REG(port, pin)  (0x001c + 0x24 * (port) + 4 * ((pin) / PULL_PPW))
#define PULL_BIT(pin)        (PULL_WIDTH * ((pin) % PULL_PPW))

#define PINS_PER_PORT        32
#define GET_PORT(gpio)       ((gpio)->id / PINS_PER_PORT)
#define GET_PIN(gpio)        ((gpio)->id % PINS_PER_PORT)

static int
sunxi_gpio_get_value(const struct gpio_handle *gpio, bool *value)
{
	const struct simple_device *self = to_simple_device(gpio->dev);
	uint8_t port   = GET_PORT(gpio);
	uint8_t pin    = GET_PIN(gpio);
	uintptr_t regs = self->regs;

	*value = mmio_get_bitfield_32(regs + DATA_REG(port), DATA_BIT(pin),
	                              DATA_WIDTH);

	return SUCCESS;
}

static int
sunxi_gpio_init_pin(const struct gpio_handle *gpio)
{
	const struct simple_device *self = to_simple_device(gpio->dev);
	uint8_t port   = GET_PORT(gpio);
	uint8_t pin    = GET_PIN(gpio);
	uintptr_t regs = self->regs;

	/* Set pin function configuration (mode). */
	mmio_set_bitfield_32(regs + MODE_REG(port, pin), MODE_BIT(pin),
	                     MODE_WIDTH, gpio->mode);
	/* Set pin drive strength. */
	mmio_set_bitfield_32(regs + DRIVE_REG(port, pin), DRIVE_BIT(pin),
	                     DRIVE_WIDTH, gpio->drive);
	/* Set pin pull-up or pull-down. */
	mmio_set_bitfield_32(regs + PULL_REG(port, pin), PULL_BIT(pin),
	                     PULL_WIDTH, gpio->pull);

	return SUCCESS;
}

static int
sunxi_gpio_set_value(const struct gpio_handle *gpio, bool value)
{
	const struct simple_device *self = to_simple_device(gpio->dev);
	uint8_t port   = GET_PORT(gpio);
	uint8_t pin    = GET_PIN(gpio);
	uintptr_t regs = self->regs;

	mmio_set_bitfield_32(regs + DATA_REG(port), DATA_BIT(pin),
	                     DATA_WIDTH, value);

	return SUCCESS;
}

static const struct gpio_driver sunxi_gpio_driver = {
	.drv = {
		.probe   = simple_device_probe,
		.release = simple_device_release,
	},
	.ops = {
		.get_value = sunxi_gpio_get_value,
		.init_pin  = sunxi_gpio_init_pin,
		.set_value = sunxi_gpio_set_value,
	},
};

const struct simple_device pio = {
	.dev = {
		.name  = "pio",
		.drv   = &sunxi_gpio_driver.drv,
		.state = DEVICE_STATE_INIT,
	},
	.clock = { .dev = &ccu.dev, .id = CLK_BUS_PIO },
	.regs  = DEV_PIO,
};

const struct simple_device r_pio = {
	.dev = {
		.name  = "r_pio",
		.drv   = &sunxi_gpio_driver.drv,
		.state = DEVICE_STATE_INIT,
	},
	.clock = { .dev = &r_ccu.dev, .id = CLK_BUS_R_PIO },
	.regs  = DEV_R_PIO,
};
