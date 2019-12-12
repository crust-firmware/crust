/*
 * Copyright © 2017-2019 The Crust Firmware Authors.
 * SPDX-License-Identifier: BSD-3-Clause OR GPL-2.0-only
 */

#ifndef COMMON_DM_H
#define COMMON_DM_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <util.h>

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
	/** A function called to check for new work or state changes. */
	void (*poll)(const struct device *dev);
	/** A function called to detect and initialize new devices. */
	int  (*probe)(const struct device *dev);
};

/**
 * Ensure a device is probed (or in other words, its driver is initialized).
 * If a device's driver is already initialized, this function does nothing.
 *
 * @param dev  A device
 */
void device_probe(const struct device *dev);

#endif /* COMMON_DM_H */
