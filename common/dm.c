/*
 * Copyright © 2017-2018 The Crust Firmware Authors.
 * SPDX-License-Identifier: (BSD-3-Clause OR GPL-2.0)
 */

#include <debug.h>
#include <dm.h>
#include <error.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

extern struct device device_list[];
extern struct device device_list_end[];

static int
device_probe(struct device *dev)
{
	int err;

	assert(dev->drv);
	assert(dev->drv->probe);

	/* Skip already-probed devices. */
	if (dev->flags & DEVICE_FLAG_RUNNING)
		return SUCCESS;
	if (dev->flags & DEVICE_FLAG_MISSING)
		return ENODEV;

	/* Probe all devices this device depends on. */
	if (dev->bus && (err = device_probe(dev->bus)))
		return err;
	if (dev->clockdev && (err = device_probe(dev->clockdev)))
		return err;
	if (dev->irqdev && (err = device_probe(dev->irqdev)))
		return err;
	if (dev->supplydev && (err = device_probe(dev->supplydev)))
		return err;

	/* Probe the device itself, and report any errors. */
	if ((err = dev->drv->probe(dev)) == SUCCESS) {
		dev->flags |= DEVICE_FLAG_RUNNING;
		debug("dm: Probed %s", dev->name);
	} else if (err == ENODEV) {
		dev->flags |= DEVICE_FLAG_MISSING;
		warn("dm: Failed to probe %s (missing)", dev->name);
	} else {
		panic("dm: Failed to probe %s (%d)", dev->name, err);
	}

	return err;
}

struct device *
dm_get_by_class(uint32_t class)
{
	for (struct device *dev = device_list; dev < device_list_end; ++dev) {
		if (!(dev->flags & DEVICE_FLAG_RUNNING))
			continue;
		if (dev->drv->class == class)
			return dev;
	}

	panic("dm: No device for class %u", class);
}

struct device *
dm_get_by_name(const char *name)
{
	for (struct device *dev = device_list; dev < device_list_end; ++dev) {
		if (!(dev->flags & DEVICE_FLAG_RUNNING))
			continue;
		if (!strcmp(dev->name, name))
			return dev;
	}

	panic("dm: Device %s not found", name);
}

void
dm_init(void)
{
	for (struct device *dev = device_list; dev < device_list_end; ++dev)
		device_probe(dev);
}
