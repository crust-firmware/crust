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

enum {
	DM_CLASS_NONE = 0,
	DM_CLASS_CLOCK,
	DM_CLASS_GPIO,
	DM_CLASS_I2C,
	DM_CLASS_IRQ,
	DM_CLASS_MSGBOX,
	DM_CLASS_PMIC,
	DM_CLASS_REGULATOR,
	DM_CLASS_RSB,
	DM_CLASS_WATCHDOG,
	DM_CLASS_COUNT,
};

struct driver;

struct device {
	/** A unique name for this device. */
	const char *const          name;
	/** The address of a block of MMIO registers (if one is used). */
	const uintptr_t            regs;
	/** The driver for this device. */
	const struct driver *const drv;
	/** The controller for the bus this device is connected to. */
	struct device *const       bus;
	/** The clocks utilized by this device. */
	struct clock_handle *const clocks;
	/** The GPIO pins utilized by this device. */
	struct gpio_handle *const  pins;
	/** The controller for this device's power supply (regulator). */
	struct device *const       supplydev;
	/** A bus-specific address/port (if this device is on a bus). */
	const uint8_t              addr;
	/** A supplydev-specific power supply identifier. */
	const uint8_t              supply;
	/** Flags describing this device's state. */
	uint8_t                    flags;
};

struct driver {
	/** One of the enumerated driver classes. */
	uint32_t class;
	/** A function called to check for new work or state changes. */
	void     (*poll)(struct device *dev);
	/** A function called to detect and initialize new devices. */
	int      (*probe)(struct device *dev);
};

/**
 * Ensure a device is probed (or in other words, its driver is initialized).
 * If a device's driver is already initialized, this function does nothing.
 *
 * @param dev  A device
 */
void device_probe(struct device *dev);

#endif /* COMMON_DM_H */
