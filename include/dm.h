/*
 * Copyright © 2017 Samuel Holland <samuel@sholland.org>
 * SPDX-License-Identifier: (BSD-3-Clause OR GPL-2.0)
 */

#ifndef DM_H
#define DM_H

#include <stdint.h>
#include <util.h>

#define __device __attribute__((section(".device"), used))

enum {
	DEVICE_FLAG_RUNNING = BIT(0),
};

enum {
	DM_CLASS_NONE = 0,
	DM_CLASS_CLOCK,
	DM_CLASS_IRQCHIP,
	DM_CLASS_MSGBOX,
};

struct driver;

struct device {
	const char          *name;
	struct device       *bus;
	uintptr_t            addr;
	uintptr_t            regs;
	uintptr_t            clock;
	struct device       *clockdev;
	const struct driver *drv;
	uintptr_t            drvdata;
	uintptr_t            irq;
	struct device       *irqdev;
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

#endif /* DM_H */
