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
};

enum {
	DM_CLASS_NONE = 0,
	DM_CLASS_CLOCK,
	DM_CLASS_GPIO,
	DM_CLASS_I2C,
	DM_CLASS_IRQCHIP,
	DM_CLASS_MSGBOX,
	DM_CLASS_PIO,
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
	const char *name;
	uint32_t    class;
	int         (*probe)(struct device *dev);
	const void *ops;
};

struct device *dm_get_by_class(uint32_t class);
struct device *dm_get_by_name(const char *name);
void dm_init(void);

#endif /* COMMON_DM_H */
