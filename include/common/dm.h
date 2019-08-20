/*
 * Copyright © 2017-2019 The Crust Firmware Authors.
 * SPDX-License-Identifier: BSD-3-Clause OR GPL-2.0-only
 */

#ifndef COMMON_DM_H
#define COMMON_DM_H

#include <stdint.h>

struct driver;

struct device {
	/** A unique name for this device. */
	const char *const          name;
	/** The address of a block of MMIO registers (if one is used). */
	const uintptr_t            regs;
	/** The driver for this device. */
	const struct driver *const drv;
};

struct driver {
	/** A function called to check for new work or state changes. */
	void (*poll)(struct device *dev);
	/** A function called to detect and initialize new devices. */
	int  (*probe)(struct device *dev);
};

#endif /* COMMON_DM_H */
