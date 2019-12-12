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

enum {
	DEVICE_FLAG_RUNNING = BIT(0),
};

struct driver;

struct device {
	/** A unique name for this device. */
	const char *const          name;
	/** The driver for this device. */
	const struct driver *const drv;
	/** Flags describing this device's state. */
	uint8_t                    flags;
};

struct driver {
	/** A function called to check for new work or state changes. */
	void (*poll)(struct device *dev);
	/** A function called to detect and initialize new devices. */
	int  (*probe)(struct device *dev);
};

/**
 * Ensure a device is probed (or in other words, its driver is initialized).
 * If a device's driver is already initialized, this function does nothing.
 *
 * @param dev  A device
 */
void device_probe(struct device *dev);

#endif /* COMMON_DM_H */
