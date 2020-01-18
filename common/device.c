/*
 * Copyright © 2017-2020 The Crust Firmware Authors.
 * SPDX-License-Identifier: BSD-3-Clause OR GPL-2.0-only
 */

#include <debug.h>
#include <device.h>
#include <error.h>
#include <stddef.h>

const struct device *
device_get(const struct device *dev)
{
	int err;

	if (!dev)
		return NULL;

	if (!dev->state->refcount) {
		debug("%s: Probing", dev->name);
		if ((err = dev->drv->probe(dev))) {
			error("%s: Probe failed: %d", dev->name, err);
			return NULL;
		}
	}

	/* Increment the refcount only after successful initialization. */
	++dev->state->refcount;

	return dev;
}

void
device_put(const struct device *dev)
{
	if (--dev->state->refcount)
		return;

	debug("%s: Releasing", dev->name);
	dev->drv->release(dev);
}

int
dummy_probe(const struct device *dev UNUSED)
{
	return SUCCESS;
}

void
dummy_release(const struct device *dev UNUSED)
{
}
