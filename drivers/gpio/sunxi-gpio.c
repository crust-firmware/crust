/*
 * Copyright © 2017-2019 The Crust Firmware Authors.
 * SPDX-License-Identifier: BSD-3-Clause OR GPL-2.0-only
 */

#include <device.h>
#include <error.h>
#include <gpio.h>
#include <mmio.h>
#include <stdbool.h>
#include <stddef.h>
#include <util.h>
#include <clock/sunxi-ccu.h>
#include <gpio/sunxi-gpio.h>
#include <platform/devices.h>

#define CONFIG_REG(port, index) (0x0000 + 0x24 * (port) + ((index) / 8) * 4)
#define CONFIG_OFFSET(index)    (((pin) % 8) * 4)
#define CONFIG_MASK(index)      GENMASK(CONFIG_OFFSET(index) + 2, \
	                                CONFIG_OFFSET(index))

#define DATA_REG(port)          (0x0010 + 0x24 * (port))

#define DRIVE_REG(port, index)  (0x0014 + 0x24 * (port) + ((index) / 16) * 2)

#define PULL_REG(port, index)   (0x001c + 0x24 * (port) + ((index) / 16) * 2)

#define PINS_PER_PORT           32
#define PIN_INDEX(pin)          ((pin) % PINS_PER_PORT)
#define PIN_PORT(pin)           ((pin) / PINS_PER_PORT)

/* The most implemented ports on any supported device. */
#define MAX_PORTS               8

static inline const struct sunxi_gpio *
to_sunxi_gpio(const struct device *dev)
{
	return container_of(dev, const struct sunxi_gpio, dev);
}

static int
sunxi_gpio_get_value(const struct device *dev, uint8_t pin, bool *value)
{
	const struct sunxi_gpio *self = to_sunxi_gpio(dev);
	uint8_t index = PIN_INDEX(pin);
	uint8_t port  = PIN_PORT(pin);

	*value = mmio_read_32(self->regs + DATA_REG(port)) & BIT(index);

	return SUCCESS;
}

static int
sunxi_gpio_set_mode(const struct device *dev, uint8_t pin, uint8_t mode)
{
	const struct sunxi_gpio *self = to_sunxi_gpio(dev);
	uint8_t index = PIN_INDEX(pin);
	uint8_t port  = PIN_PORT(pin);

	/* Set pin function configuration. */
	mmio_clrset_32(self->regs + CONFIG_REG(port, index),
	               CONFIG_MASK(index), mode << CONFIG_OFFSET(index));

	return SUCCESS;
}

static int
sunxi_gpio_set_value(const struct device *dev, uint8_t pin, bool value)
{
	const struct sunxi_gpio *self = to_sunxi_gpio(dev);
	uint8_t index = PIN_INDEX(pin);
	uint8_t port  = PIN_PORT(pin);

	/* Set the pin to the specified value. */
	mmio_clrset_32(self->regs + DATA_REG(port),
	               BIT(index), value << index);

	return SUCCESS;
}

static int
sunxi_gpio_probe(const struct device *dev)
{
	const struct sunxi_gpio *self = to_sunxi_gpio(dev);
	int err;

	if ((err = clock_get(&self->clock)))
		return err;

	return SUCCESS;
}

static void
sunxi_gpio_release(const struct device *dev)
{
	const struct sunxi_gpio *self = to_sunxi_gpio(dev);

	clock_put(&self->clock);
}

static const struct gpio_driver sunxi_gpio_driver = {
	.drv = {
		.probe   = sunxi_gpio_probe,
		.release = sunxi_gpio_release,
	},
	.ops = {
		.get_value = sunxi_gpio_get_value,
		.set_mode  = sunxi_gpio_set_mode,
		.set_value = sunxi_gpio_set_value,
	},
};

const struct sunxi_gpio r_pio = {
	.dev = {
		.name  = "r_pio",
		.drv   = &sunxi_gpio_driver.drv,
		.state = DEVICE_STATE_INIT,
	},
	.clock = { .dev = &r_ccu.dev, .id = CLK_BUS_R_PIO },
	.regs  = DEV_R_PIO,
};
