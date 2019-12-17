/*
 * Copyright © 2017-2019 The Crust Firmware Authors.
 * SPDX-License-Identifier: BSD-3-Clause OR GPL-2.0-only
 */

#include <debug.h>
#include <device.h>
#include <stddef.h>

const struct device *
device_get(const struct device *dev)
{
	int err;

	/* Skip already-running devices. */
	if (device_is_running(dev))
		return dev;

	debug("%s: Probing", dev->name);
	if ((err = dev->drv->probe(dev))) {
		error("%s: Probe failed: %d", dev->name, err);
		return NULL;
	}

	/* Only increase the refcount if probing succeeded. */
	++dev->state->refcount;

	return dev;
}

bool
device_is_running(const struct device *dev)
{
	return dev->state->refcount;
}

void
device_put(const struct device *dev)
{
	--dev->state->refcount;
}
