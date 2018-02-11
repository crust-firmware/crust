/*
 * Copyright © 2017-2018 The Crust Firmware Authors.
 * SPDX-License-Identifier: (BSD-3-Clause OR GPL-2.0)
 */

#ifndef DRIVERS_WALLCLOCK_H
#define DRIVERS_WALLCLOCK_H

#include <dm.h>
#include <stdint.h>

#define WALLCLOCK_OPS(dev) ((struct wallclock_driver_ops *)((dev)->drv->ops))

struct wallclock_driver_ops {
	uint64_t (*read)(struct device *dev);
};

int wallclock_device_register(struct device *dev);
uint64_t wallclock_read(void);

#endif /* DRIVERS_WALLCLOCK_H */
