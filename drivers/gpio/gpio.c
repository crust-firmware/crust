/*
 * Copyright © 2017-2019 The Crust Firmware Authors.
 * SPDX-License-Identifier: BSD-3-Clause OR GPL-2.0-only
 */

#include <device.h>
#include <error.h>
#include <gpio.h>

int
gpio_get(const struct gpio_handle *gpio)
{
	const struct device *dev = gpio->dev;
	uint8_t pin  = gpio->pin;
	uint8_t mode = gpio->mode;
	int err;

	/* Ensure the controller's driver is loaded. */
	device_probe(dev);

	/* Set the GPIO pin mode. */
	if ((err = gpio_set_mode(dev, pin, mode)))
		return err;

	return SUCCESS;
}
