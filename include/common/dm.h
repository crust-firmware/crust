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

#define DEVICE_HANDLE_INIT(cls) \
	(struct device_handle) { \
		.class = (cls), \
		.index = -1, \
	}

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

struct device_handle {
	/** Pointer to the device. */
	struct device *dev;
	/** Subdevice ID within the device (if used). */
	uint8_t        id;
	/** Class of the device (used for iteration). */
	uint8_t        class;
	/** Index of the device in the device list (used for iteration). */
	int8_t         index;
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

/**
 * Get the first device of a given class. This function only considers
 * successfully-probed devices.
 *
 * @param class One of the enumerated driver classes.
 * @return      The address of the device description, or NULL if no
 *              devices of this class were found.
 */
struct device *dm_first_dev_by_class(uint32_t class);

/**
 * Get the next device of a given class. This function only considers
 * successfully-probed devices.
 *
 * @param handle Pointer to a handle initialized with DEVICE_HANDLE_INIT or a
 *               previous successful call to a driver model getter or iterator.
 * @return       Zero if a device was found and the handle was updated. Error
 *               if no device exists, and the handle contents are undefined.
 */
int dm_next_dev_by_class(struct device_handle *handle);

/**
 * Initialize the driver model, probing all devices in topological order.
 * If a device cannot be probed, the firmware will panic.
 */
void dm_init(void);

/**
 * Poll all devices that have the `.poll` hook populated in their driver.
 */
void dm_poll(void);

/**
 * Set up the clocks specified for a device.
 *
 * @param dev        The device referencing the clocks to initialize.
 * @param num_clocks The number of clocks utilized by the device.
 */
int dm_setup_clocks(struct device *dev, uint8_t num_clocks);

/**
 * Set the mode of the GPIO pins specified for a device.
 *
 * @param dev       The device referencing the GPIO pins to initialize.
 * @param num_pins  The number of pins utilized by the device.
 */
int dm_setup_pins(struct device *dev, uint8_t num_pins);

#endif /* COMMON_DM_H */
