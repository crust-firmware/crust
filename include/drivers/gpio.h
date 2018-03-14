/*
 * Copyright © 2017-2018 The Crust Firmware Authors.
 * SPDX-License-Identifier: (BSD-3-Clause OR GPL-2.0)
 */

#ifndef DRIVERS_GPIO_H
#define DRIVERS_GPIO_H

#include <dm.h>
#include <stdbool.h>
#include <stdint.h>

#define GPIO_OPS(dev) ((struct gpio_driver_ops *)((dev)->drv->ops))

struct gpio_driver_ops {
	bool (*read_pin)(struct device *dev, uint8_t pin);
	int  (*set_mode)(struct device *dev, uint8_t pin, uint8_t mode);
	int  (*write_pin)(struct device *dev, uint8_t pin, bool val);
};

struct gpio_pins {
	struct device *dev; /**< The device that utilizes the GPIO pins. */
	uint8_t        count;
	struct {
		uint8_t id;
		uint8_t mode;
	} pins[];
};

/**
 * Read the value of a pin.
 *
 * @param dev The port I/O device.
 * @param pin The index of the pin to retrieve the value for.
 */
bool gpio_read_pin(struct device *dev, uint8_t pin);

/**
 * Sets the mode of all pins registered in the device description.
 *
 * @param dev  The port I/O device.
 */
int gpio_set_pins(struct device *dev);

/**
 * Write the value of a pin.
 *
 * @param dev The port I/O device.
 * @param pin The index of the pin in the device specification to write to.
 * @param val The value to write to the specified pin.
 */
int gpio_write_pin(struct device *dev, uint8_t pin, bool val);

#endif /* DRIVERS_GPIO_H */
