/*
 * Copyright © 2017-2018 The Crust Firmware Authors.
 * SPDX-License-Identifier: BSD-3-Clause OR GPL-2.0-only
 */

#ifndef DRIVERS_GPIO_H
#define DRIVERS_GPIO_H

#include <dm.h>
#include <intrusive.h>
#include <stdbool.h>
#include <stdint.h>

#define GPIO_OPS(dev) \
	(&container_of((dev)->drv, struct gpio_driver, drv)->ops)

#define GPIO_PINS(n) (struct gpio_handle[n])

struct gpio_driver_ops {
	int (*get_value)(struct device *dev, uint8_t pin, bool *value);
	int (*set_mode)(struct device *dev, uint8_t pin, uint8_t mode);
	int (*set_value)(struct device *dev, uint8_t pin, bool value);
};

struct gpio_driver {
	const struct driver          drv;
	const struct gpio_driver_ops ops;
};

struct gpio_handle {
	struct device *const dev;
	const uint8_t        pin;
	const uint8_t        mode;
};

/**
 * Get the value of a pin.
 *
 * @param dev   The GPIO controller.
 * @param pin   The identifier of the pin to retrieve the value for.
 * @param value The location to store the value read from the specified pin.
 */
static inline int
gpio_get_value(struct device *dev, uint8_t pin, bool *value)
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
gpio_set_mode(struct device *dev, uint8_t pin, uint8_t mode)
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
gpio_set_value(struct device *dev, uint8_t pin, bool value)
{
	return GPIO_OPS(dev)->set_value(dev, pin, value);
}

#endif /* DRIVERS_GPIO_H */
