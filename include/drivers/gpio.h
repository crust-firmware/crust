/*
 * Copyright © 2017-2019 The Crust Firmware Authors.
 * SPDX-License-Identifier: BSD-3-Clause OR GPL-2.0-only
 */

#ifndef DRIVERS_GPIO_H
#define DRIVERS_GPIO_H

#include <device.h>
#include <intrusive.h>
#include <stdbool.h>
#include <stdint.h>

#define GPIO_OPS(dev) \
	(&container_of((dev)->drv, const struct gpio_driver, drv)->ops)

struct gpio_handle {
	const struct device *dev;
	uint8_t              pin;
	uint8_t              mode;
};

struct gpio_driver_ops {
	int (*get_value)(const struct device *dev, uint8_t pin, bool *value);
	int (*set_mode)(const struct device *dev, uint8_t pin, uint8_t mode);
	int (*set_value)(const struct device *dev, uint8_t pin, bool value);
};

struct gpio_driver {
	struct driver          drv;
	struct gpio_driver_ops ops;
};

/**
 * Get a reference to a pin and its controller device, and set the pin's mode.
 *
 * @param gpio A handle for the GPIO pin.
 */
int gpio_get(const struct gpio_handle *gpio);

/**
 * Get the value of a pin.
 *
 * @param dev   The GPIO controller.
 * @param pin   The identifier of the pin to retrieve the value for.
 * @param value The location to store the value read from the specified pin.
 */
static inline int
gpio_get_value(const struct device *dev, uint8_t pin, bool *value)
{
	return GPIO_OPS(dev)->get_value(dev, pin, value);
}

/**
 * Set the mode of a pin.
 *
 * @param dev  The GPIO controller.
 * @param pin  The identifier of the pin to set the mode of.
 * @param mode The mode to set for the specified pin.
 */
static inline int
gpio_set_mode(const struct device *dev, uint8_t pin, uint8_t mode)
{
	return GPIO_OPS(dev)->set_mode(dev, pin, mode);
}

/**
 * Set the value of a pin.
 *
 * @param dev   The GPIO controller.
 * @param pin   The identifier of the pin to set the value of.
 * @param value The value to set for the specified pin.
 */
static inline int
gpio_set_value(const struct device *dev, uint8_t pin, bool value)
{
	return GPIO_OPS(dev)->set_value(dev, pin, value);
}

#endif /* DRIVERS_GPIO_H */
