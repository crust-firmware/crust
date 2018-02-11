/*
 * Copyright © 2017-2018 The Crust Firmware Authors.
 * SPDX-License-Identifier: (BSD-3-Clause OR GPL-2.0)
 */

#include <dm.h>
#include <error.h>
#include <drivers/wallclock.h>

static struct device *wallclock;

int
wallclock_device_register(struct device *dev)
{
	if (wallclock)
		return EEXIST;
	wallclock = dev;

	return SUCCESS;
}

uint64_t
wallclock_read(void)
{
	return WALLCLOCK_OPS(wallclock)->read(wallclock);
}
