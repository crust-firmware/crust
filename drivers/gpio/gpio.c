/*
 * Copyright © 2017-2018 The Crust Firmware Authors.
 * SPDX-License-Identifier: (BSD-3-Clause OR GPL-2.0)
 */

#include <debug.h>
#include <error.h>
#include <gpio.h>
#include <stddef.h>

bool
gpio_read_pin(struct device *dev, uint8_t pin)
{
	struct gpio_pins *gpio     = dev->gpio;
	struct device    *gpio_dev = gpio->dev;
	uint8_t pin_id;

	assert(pin < gpio->count);

	/* Retrieve the pin ID from the device specification. */
	pin_id = gpio->pins[pin].id;

	return GPIO_OPS(gpio_dev)->read_pin(gpio_dev, pin_id);
}

int
gpio_set_pins(struct device *dev)
{
	struct gpio_pins *gpio = dev->gpio;
	struct device    *gpio_dev = gpio->dev;
	uint8_t num_pins, pin_id, mode;
	int     result = EINVAL;

	num_pins = gpio->count;
	assert(num_pins > 0);

	for (size_t p = 0; p < num_pins; ++p) {
		pin_id = gpio->pins[p].id;
		mode   = gpio->pins[p].mode;

		if (GPIO_OPS(gpio_dev)->set_mode(gpio_dev, pin_id, mode))
			break;
	}

	return result;
}

int
gpio_write_pin(struct device *dev, uint8_t pin, bool val)
{
	struct gpio_pins *gpio     = dev->gpio;
	struct device    *gpio_dev = gpio->dev;
	uint8_t pin_id;

	assert(pin < gpio->count);

	/* Retrieve the pin ID from the device specification. */
	pin_id = gpio->pins[pin].id;

	return GPIO_OPS(gpio_dev)->write_pin(gpio_dev, pin_id, val);
}
