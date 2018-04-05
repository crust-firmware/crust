/*
 * Copyright © 2017-2018 The Crust Firmware Authors.
 * SPDX-License-Identifier: (BSD-3-Clause OR GPL-2.0)
 */

#include <debug.h>
#include <dm.h>
#include <error.h>
#include <gpio.h>
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
	struct device *dev;

	for (dev = device_list; dev < device_list_end; ++dev) {
		if (!(dev->flags & DEVICE_FLAG_RUNNING))
			continue;
		if (dev->drv->class == class)
			return dev;
	}

	return NULL;
}

struct device *
dm_get_by_name(const char *name)
{
	struct device *dev;

	for (dev = device_list; dev < device_list_end; ++dev) {
		if (!(dev->flags & DEVICE_FLAG_RUNNING))
			continue;
		if (!strcmp(dev->name, name))
			return dev;
	}

	return NULL;
}

void
dm_init(void)
{
	for (struct device *dev = device_list; dev < device_list_end; ++dev)
		device_probe(dev);
}

int
dm_setup_pins(struct device *dev, uint8_t num_pins)
{
	struct gpio_handle *pins = dev->gpio_pins;
	struct device *gpio_dev;
	uint8_t pin_id, mode;
	int     err;

	for (size_t i = 0; i < num_pins; ++i) {
		gpio_dev = pins[i].dev;
		pin_id   = pins[i].pin;
		mode     = pins[i].mode;

		/* Probe to ensure GPIO controller is loaded. */
		if ((err = device_probe(gpio_dev)))
			return err;

		/* Set pin mode and return if error occurs. */
		if ((err = gpio_set_mode(gpio_dev, pin_id, mode)))
			return err;
	}

	return SUCCESS;
}
