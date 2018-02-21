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
	DM_CLASS_I2C,
	DM_CLASS_IRQCHIP,
	DM_CLASS_MSGBOX,
	DM_CLASS_PIO,
	DM_CLASS_TIMER,
	DM_CLASS_WATCHDOG,
};

struct driver;

struct device {
	/** A unique name for the device (used for debugging) */
	const char          *name;
	/** The parent bus/controller (usually NULL for MMIO devices) */
	struct device       *bus;
	/** A bus-specific address/pin/port (if on a bus) */
	uintptr_t            addr;
	/** Beginning of an MMIO register block (if one exists) */
	uintptr_t            regs;
	/** A clockdev-specific clock description */
	uintptr_t            clock;
	/** The controller for the device's clock(s) */
	struct device       *clockdev;
	/** The driver for this device */
	const struct driver *drv;
	/** Extra per-device driver-specific data */
	uintptr_t            drvdata;
	/** An irqdev-specific IRQ description */
	uintptr_t            irq;
	/** The controller for the device's IRQ(s) */
	struct device       *irqdev;
	/** Flags describing the device's state */
	uint8_t              flags;
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
