/*
 * Copyright © 2017-2019 The Crust Firmware Authors.
 * SPDX-License-Identifier: BSD-3-Clause OR GPL-2.0-only
 */

#include <debug.h>
#include <dm.h>

void
device_probe(struct device *dev)
{
	int err;

	/* Skip already-probed devices. */
	if (dev->flags & DEVICE_FLAG_RUNNING)
		return;

	/* Probe all devices this device depends on. */
	if (dev->bus)
		device_probe(dev->bus);
	if (dev->supplydev)
		device_probe(dev->supplydev);

	/* Probe the device itself, and report any errors. */
	if ((err = dev->drv->probe(dev)))
		panic("dm: Failed to probe %s (%d)", dev->name, err);

	dev->flags |= DEVICE_FLAG_RUNNING;

	debug("dm: Probed %s", dev->name);
}
