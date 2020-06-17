/*
 * Copyright © 2017-2020 The Crust Firmware Authors.
 * SPDX-License-Identifier: BSD-3-Clause OR GPL-2.0-only
 */

#ifndef COMMON_DEVICE_H
#define COMMON_DEVICE_H

#include <stdbool.h>
#include <stdint.h>

/**
 * Default initializer for the device state pointer.
 *
 * The state pointer must be initialized for all devices. If additional mutable
 * state is needed, wrap struct device_state intrusively. Otherwise, use this
 * macro.
 */
#define DEVICE_STATE_INIT &(struct device_state) { 0 }

struct device_state;
struct driver;

struct device {
	/** A unique name for this device. */
	const char          *name;
	/** The driver for this device. */
	const struct driver *drv;
	/** Mutable state for this device. */
	struct device_state *state;
};

struct device_state {
	/** Reference count for this device. */
	uint8_t refcount;
};

struct driver {
	/** A function called to detect and initialize new devices. */
	int  (*probe)(const struct device *dev);
	/** A function called to uninitialize devices and free resources. */
	void (*release)(const struct device *dev);
};

/**
 * Get a reference to a device.
 *
 * If this is the first reference to a device, that device's driver will be
 * initialized. Otherwise, this function only updates the reference count.
 *
 * The device will remain running as long as the reference is held (that is,
 * until calling device_put()).
 *
 * If this function returns an error, do not call device_put().
 *
 * @param dev A device.
 *
 * @return Zero on success; an error code on failure.
 */
int device_get(const struct device *dev);

/**
 * Get a reference to a device.
 *
 * @return A reference to the device that was acquired, or a NULL pointer.
 */
const struct device *device_get_or_null(const struct device *dev);

/**
 * Release a reference to a device.
 *
 * @param dev A reference to a device.
 */
void device_put(const struct device *dev);

/**
 * Implementation of the device probe function that does nothing.
 */
int dummy_probe(const struct device *dev);

/**
 * Implementation of the device release function that does nothing.
 */
void dummy_release(const struct device *dev);

#endif /* COMMON_DEVICE_H */
