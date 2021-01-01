/*
 * Copyright © 2017-2021 The Crust Firmware Authors.
 * SPDX-License-Identifier: BSD-3-Clause OR GPL-2.0-only
 */

#ifndef DRIVERS_GPIO_H
#define DRIVERS_GPIO_H

#include <device.h>
#include <stdbool.h>
#include <stdint.h>

struct gpio_handle {
	const struct device *dev;
	uint8_t              id;
	uint8_t              drive;
	uint8_t              mode;
	uint8_t              pull;
};

/**
 * Get a reference to a GPIO pin and its controller device, and set up the pin.
 *
 * The pin's mode, drive strength, and pull-up or pull-down will be set based
 * on the values in the handle.
 *
 * @param gpio A handle specifying the GPIO pin.
 * @return     Zero on success; an error code on failure.
 */
int gpio_get(const struct gpio_handle *gpio);

/**
 * Get the value of a GPIO pin.
 *
 * If the pin mode is not "input", this may not get the actual hardware state.
 *
 * @param gpio  A reference to a GPIO pin.
 * @param value The location to store the value read from the pin.
 * @return      Zero on success; an error code on failure.
 */
int gpio_get_value(const struct gpio_handle *gpio, bool *value);

/**
 * Release a reference to a GPIO pin and its controller device.
 *
 * If this is the last reference to a GPIO pin, that pin will be disabled.
 *
 * @param gpio A reference to a GPIO pin.
 */
void gpio_put(const struct gpio_handle *gpio);

/**
 * Set the value of a GPIO pin.
 *
 * If the pin mode is not "output", this may have no effect on the hardware.
 *
 * @param gpio  A reference to a GPIO pin.
 * @param value The value to set for the pin.
 * @return      Zero on success; an error code on failure.
 */
int gpio_set_value(const struct gpio_handle *gpio, bool value);

#endif /* DRIVERS_GPIO_H */
