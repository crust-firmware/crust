/*
 * Copyright © 2017-2019 The Crust Firmware Authors.
 * SPDX-License-Identifier: BSD-3-Clause OR GPL-2.0-only
 */

#include <debug.h>
#include <device.h>

void
device_probe(const struct device *dev)
{
	int err;

	/* Skip already-probed devices. */
	if (dev->state->refcount++)
		return;

	/* Probe the device itself, and report any errors. */
	if ((err = dev->drv->probe(dev)))
		panic("dm: Failed to probe %s (%d)", dev->name, err);

	debug("dm: Probed %s", dev->name);
}
