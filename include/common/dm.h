/*
 * Copyright © 2017-2018 The Crust Firmware Authors.
 * SPDX-License-Identifier: (BSD-3-Clause OR GPL-2.0)
 */

#ifndef COMMON_DM_H
#define COMMON_DM_H

#include <stdint.h>
#include <util.h>

#define __device __attribute__((section(".device"), used))

enum {
	DEVICE_FLAG_RUNNING = BIT(0),
	DEVICE_FLAG_MISSING = BIT(1),
};

enum {
	DM_CLASS_NONE = 0,
	DM_CLASS_CLOCK,
	DM_CLASS_GPIO,
	DM_CLASS_I2C,
	DM_CLASS_IRQCHIP,
	DM_CLASS_MSGBOX,
	DM_CLASS_REGULATOR,
	DM_CLASS_TIMER,
	DM_CLASS_WATCHDOG,
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
	/** The GPIO pins utilized by this device. */
	struct gpio_handle        *gpio_pins;
	/** The controller for this device's clock. */
	struct device *const       clockdev;
	/** The controller for this device's IRQ. */
	struct device *const       irqdev;
	/** The controller for this device's power supply (regulator). */
	struct device *const       supplydev;
	/** A bus-specific address/port (if this device is on a bus). */
	const uint8_t              addr;
	/** A clockdev-specific clock identifier. */
	const uint8_t              clock;
	/** An irqdev-specific IRQ number. */
	const uint8_t              irq;
	/** A supplydev-specific power supply identifier. */
	const uint8_t              supply;
	/** Flags describing about this device's state. */
	uint8_t                    flags;
};

struct driver {
	/** A unique name for this driver. */
	const char *const name;
	/** One of the enumerated driver classes. */
	const uint32_t    class;
	/** A function called to detect and initialize new devices. */
	int               (*probe)(struct device *dev);
};

/**
 * Get the first device of a given class.
 *
 * @param class One of the enumerated driver classes.
 */
struct device *dm_get_by_class(uint32_t class);

/**
 * Get the device with the given name.
 *
 * @param name The name of the device.
 */
struct device *dm_get_by_name(const char *name);

/**
 * Initialize the driver model, probing all devices in topological order.
 */
void dm_init(void);

/**
 * Set the mode of the GPIO pins specified for a device.
 *
 * @param dev       The device containing the GPIO pins to initialize.
 * @param num_pins  The number of pins utilized by the device.
 */
int dm_setup_pins(struct device *dev, uint8_t num_pins);

#endif /* COMMON_DM_H */
