/*
 * Copyright © 2017-2020 The Crust Firmware Authors.
 * SPDX-License-Identifier: BSD-3-Clause OR GPL-2.0-only
 */

#ifndef COMMON_SIMPLE_DEVICE_H
#define COMMON_SIMPLE_DEVICE_H

#include <clock.h>
#include <device.h>
#include <gpio.h>
#include <intrusive.h>

#define SIMPLE_DEVICE_PINS      2
#define SIMPLE_DEVICE_PINS_INIT (const struct gpio_handle[SIMPLE_DEVICE_PINS])

struct simple_device {
	struct device             dev;
	struct clock_handle       clock;
	const struct gpio_handle *pins;
	uintptr_t                 regs;
};

/**
 * Downcast a pointer to a simple device.
 */
static inline const struct simple_device *
to_simple_device(const struct device *dev)
{
	return container_of(dev, const struct simple_device, dev);
}

/**
 * Probe a simple device, which uses a clock and 0-2 GPIO pins.
 *
 * This function can be used to implement a device's .probe hook.
 */
int simple_device_probe(const struct device *dev);

/**
 * Release a simple device.
 *
 * This function can be used to implement a device's .release hook.
 */
void simple_device_release(const struct device *dev);

/**
 * Adjust the refcount of a device to match the state Linux left it in.
 *
 * If a device has no references, but its clock is running, acquire a
 * persistent reference to prevent the device and its resources from being
 * released.
 *
 * Similarly, if the only reference to a device is one previously acquired by
 * this function, and the device's clock is not running, release the reference.
 *
 * This function must not be called if other references to the device are held.
 */
void simple_device_sync(const struct simple_device *sdev);

#endif /* COMMON_SIMPLE_DEVICE_H */
