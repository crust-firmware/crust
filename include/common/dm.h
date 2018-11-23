/*
 * Copyright © 2017-2018 The Crust Firmware Authors.
 * SPDX-License-Identifier: BSD-3-Clause OR GPL-2.0-only
 */

#ifndef COMMON_DM_H
#define COMMON_DM_H

#include <stddef.h>
#include <stdint.h>
#include <util.h>
#include <work.h>

#define __device __attribute__((section(".device"), used))

#define for_each_dev_in_class(var, class) \
	for (struct device *var = dm_first_dev_by_class(class); \
	     var != NULL; var = dm_next_dev_by_class(class, var))

enum {
	DEVICE_FLAG_RUNNING = BIT(0),
	DEVICE_FLAG_MISSING = BIT(1),
};

enum {
	DM_CLASS_NONE = 0,
	DM_CLASS_CLOCK,
	DM_CLASS_DVFS,
	DM_CLASS_GPIO,
	DM_CLASS_I2C,
	DM_CLASS_IRQCHIP,
	DM_CLASS_MSGBOX,
	DM_CLASS_PMIC,
	DM_CLASS_REGULATOR,
	DM_CLASS_SENSOR,
	DM_CLASS_TIMER,
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
	/** Extra per-device data private to its driver. */
	uintptr_t                  drvdata;
	/** The controller for the bus this device is connected to. */
	struct device *const       bus;
	/** The clocks utilized by this device. */
	struct clock_handle *const clocks;
	/** The IRQ line connected to this device. */
	struct irq_handle *const   irq;
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
	/** The number of subdevices (channels, etc.) within this device.
	 * Subdevice-aware drivers should update this in the probe function. */
	uint8_t                    subdev_count;
	/** The index within the class of this device's first subdevice. */
	uint8_t                    subdev_index;
};

struct driver {
	/** One of the enumerated driver classes. */
	const uint32_t class;
	/** A function called to detect and initialize new devices. */
	int            (*probe)(struct device *dev);
};

/**
 * Get the total number of subdevices (channels, etc.) available in a class.
 *
 * @param class One of the enumerated driver classes.
 * @return      The number of available subdevices.
 */
uint8_t dm_count_subdevs_by_class(uint32_t class);

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
 * @param class One of the enumerated driver classes.
 * @param prev	The previous device of this class, returned either by this
 *              function or by {@code dm_first_dev_by_class}.
 * @return      The address of the device description, or NULL if no more
 *              devices of this class were found.
 */
struct device *dm_next_dev_by_class(uint32_t class, struct device *prev);

/**
 * Get the device with the given name. This function only considers
 * successfully-probed devices.
 *
 * @param name  The name of the device.
 * @return      The address of the device description, or NULL if no device
 *              with this name was found.
 */
struct device *dm_get_dev_by_name(const char *name);

/**
 * Get the nth subdevice of a given class.
 *
 * If this function returns NULL, the value stored at {@code id} is undefined.
 *
 * @param class One of the enumerated driver classes.
 * @param index The zero-based index of the subdevice within the class.
 * @param id    An integer in which to store the per-device subdevice ID.
 * @return      The controller device containing this subdevice, or NULL if
 *              there is no subdevice with this index.
 */
struct device *dm_get_subdev_by_index(uint32_t class, uint8_t index,
                                      uint8_t *id);

/**
 * Get the next subdevice of the same class as the given subdevice.
 *
 * If this function returns NULL, the value stored at {@code id} is undefined.
 *
 * @param dev   The controller device containing the previous subdevice.
 * @param id    An integer containing the previous subdevice's per-device ID,
 *              and in which to store the next subdevice's per-device ID.
 * @return      The controller device containing this subdevice, or NULL if
 *              there is no next subdevice.
 */
struct device *dm_next_subdev(struct device *dev, uint8_t *id);

/**
 * Initialize the driver model, probing all devices in topological order.
 *
 * If a device cannot be probed because it is not present, a warning will be
 * issued. If a device cannot be probed for any other reason, the firmware will
 * panic.
 */
void dm_init(void);

/**
 * Set up the clocks specified for a device.
 *
 * @param dev        The device referencing the clocks to initialize.
 * @param num_clocks The number of clocks utilized by the device.
 */
int dm_setup_clocks(struct device *dev, uint8_t num_clocks);

/**
 * Set up the IRQ lines used by a device.
 *
 * @param dev   The device referencing the IRQs to initialize.
 * @param fn    The function to call when an IRQ is received.
 */
int dm_setup_irq(struct device *dev, callback_t *fn);

/**
 * Set the mode of the GPIO pins specified for a device.
 *
 * @param dev       The device referencing the GPIO pins to initialize.
 * @param num_pins  The number of pins utilized by the device.
 */
int dm_setup_pins(struct device *dev, uint8_t num_pins);

#endif /* COMMON_DM_H */
