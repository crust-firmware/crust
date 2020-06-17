/*
 * Copyright © 2017-2020 The Crust Firmware Authors.
 * SPDX-License-Identifier: BSD-3-Clause OR GPL-2.0-only
 */

#include <device.h>
#include <error.h>
#include <gpio.h>
#include <intrusive.h>
#include <stdint.h>

#include "gpio.h"

/**
 * Get the ops for the controller device providing this GPIO pin.
 */
static inline const struct gpio_driver_ops *
gpio_ops_for(const struct gpio_handle *gpio)
{
	const struct gpio_driver *drv =
		container_of(gpio->dev->drv, const struct gpio_driver, drv);

	return &drv->ops;
}

int
gpio_get(const struct gpio_handle *gpio)
{
	int err;

	/* Ensure the controller's driver is loaded. */
	if ((err = device_get(gpio->dev)))
		return err;

	/* Set the GPIO pin mode, drive strength, and pull-up or pull-down. */
	if ((err = gpio_ops_for(gpio)->init_pin(gpio)))
		goto err_put_device;

	return SUCCESS;

err_put_device:
	device_put(gpio->dev);

	return err;
}

int
gpio_get_value(const struct gpio_handle *gpio, bool *value)
{
	return gpio_ops_for(gpio)->get_value(gpio, value);
}

void
gpio_put(const struct gpio_handle *gpio)
{
	/* Set the GPIO pin back to its disabled state. */
	gpio_ops_for(gpio)->release_pin(gpio);

	device_put(gpio->dev);
}

int
gpio_set_value(const struct gpio_handle *gpio, bool value)
{
	return gpio_ops_for(gpio)->set_value(gpio, value);
}
