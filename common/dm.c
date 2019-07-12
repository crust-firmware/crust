/*
 * Copyright © 2017-2019 The Crust Firmware Authors.
 * SPDX-License-Identifier: BSD-3-Clause OR GPL-2.0-only
 */

#include <debug.h>
#include <devices.h>
#include <dm.h>
#include <error.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

static inline bool
device_is_running(struct device *dev)
{
	return dev->flags & DEVICE_FLAG_RUNNING;
}

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

struct device *
dm_first_dev_by_class(uint32_t class)
{
	struct device_handle handle = DEVICE_HANDLE_INIT(class);

	return dm_next_dev_by_class(&handle) == SUCCESS ? handle.dev : NULL;
}

int
dm_next_dev_by_class(struct device_handle *handle)
{
	struct device *dev;

	for (int8_t i = handle->index + 1; (dev = device_list[i]); ++i) {
		if (!device_is_running(dev))
			continue;
		if (dev->drv->class == handle->class) {
			handle->dev   = dev;
			handle->index = i;
			return SUCCESS;
		}
	}

	return ENODEV;
}

void
dm_init(void)
{
	struct device *dev;

	for (struct device *const *iter = device_list; (dev = *iter); ++iter)
		device_probe(dev);
}

void
dm_poll(void)
{
	struct device *dev;

	for (struct device *const *iter = device_list; (dev = *iter); ++iter) {
		void (*poll)(struct device *) = dev->drv->poll;
		if (poll)
			poll(dev);
	}
}
