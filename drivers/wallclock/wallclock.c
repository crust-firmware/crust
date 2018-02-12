/*
 * Copyright © 2017-2018 The Crust Firmware Authors.
 * SPDX-License-Identifier: (BSD-3-Clause OR GPL-2.0)
 */

#include <dm.h>
#include <error.h>
#include <stddef.h>
#include <drivers/wallclock.h>

static struct device *wallclock;

int
wallclock_device_register(struct device *dev)
{
	if (wallclock != NULL)
		return EEXIST;

	wallclock = dev;

	return SUCCESS;
}

uint64_t
wallclock_read(void)
{
	if (wallclock == NULL)
		return 0;

	return WALLCLOCK_OPS(wallclock)->read(wallclock);
}
