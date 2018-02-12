/*
 * Copyright © 2017-2018 The Crust Firmware Authors.
 * SPDX-License-Identifier: (BSD-3-Clause OR GPL-2.0)
 */

#ifndef DRIVERS_PIO_H
#define DRIVERS_PIO_H

#include <dm.h>
#include <stdbool.h>
#include <stdint.h>

#define PIO_OPS(dev) ((struct pio_driver_ops *)((dev)->drv->ops))

struct pio_driver_ops {
	bool (*read_pin)(struct device *dev, uint8_t pin);
	int  (*set_mode)(struct device *dev, uint8_t pin, uint8_t mode);
	int  (*write_pin)(struct device *dev, uint8_t pin, bool val);
};

/**
 * Read the value of a pin.
 *
 * @param dev The port I/O device.
 * @param pin The index of the pin to retrieve the value for.
 */
static inline bool
pio_read_pin(struct device *dev, uint8_t pin)
{
	return PIO_OPS(dev)->read_pin(dev, pin);
}

/**
 * Set the mode of a pin.
 *
 * @param dev  The port I/O device.
 * @param pin  The index of the pin to set the mode of.
 * @param mode The mode to set for the specified pin.
 */
static inline int
pio_set_mode(struct device *dev, uint8_t pin, uint8_t mode)
{
	return PIO_OPS(dev)->set_mode(dev, pin, mode);
}

/**
 * Write the value of a pin.
 *
 * @param dev The port I/O device.
 * @param pin The pin to write a value to.
 * @param val The value to write to the specified pin.
 */
static inline int
pio_write_pin(struct device *dev, uint8_t pin, bool val)
{
	return PIO_OPS(dev)->write_pin(dev, pin, val);
}

#endif /* DRIVERS_PIO_H */
