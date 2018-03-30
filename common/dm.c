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

static uint8_t total_subdevs[DM_CLASS_COUNT];

static inline bool
device_is_running(struct device *dev)
{
	return dev->flags & DEVICE_FLAG_RUNNING;
}

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

	/* Tell the device the index of its first subdevice. */
	dev->subdev_index = total_subdevs[dev->drv->class];

	/* Probe the device itself, and report any errors. */
	if ((err = dev->drv->probe(dev)) == SUCCESS) {
		dev->flags |= DEVICE_FLAG_RUNNING;
		/* If the driver's probe function did not provide the number of
		 * subdevices, assume the default number of 1. */
		if (dev->subdev_count == 0)
			dev->subdev_count = 1;
		/* Update the total number of subdevices for this class. */
		total_subdevs[dev->drv->class] += dev->subdev_count;
		debug("dm: Probed %s", dev->name);
	} else if (err == ENODEV) {
		dev->flags |= DEVICE_FLAG_MISSING;
		warn("dm: Failed to probe %s (missing)", dev->name);
	} else {
		panic("dm: Failed to probe %s (%d)", dev->name, err);
	}

	return err;
}

uint8_t
dm_count_subdevs_by_class(uint32_t class)
{
	assert(class < DM_CLASS_COUNT);

	return total_subdevs[class];
}

struct device *
dm_first_dev_by_class(uint32_t class)
{
	return dm_next_dev_by_class(class, device_list - 1);
}

struct device *
dm_next_dev_by_class(uint32_t class, struct device *prev)
{
	for (struct device *dev = prev + 1; dev < device_list_end; ++dev) {
		if (device_is_running(dev) && dev->drv->class == class)
			return dev;
	}

	return NULL;
}

struct device *
dm_get_dev_by_name(const char *name)
{
	for (struct device *dev = device_list; dev < device_list_end; ++dev) {
		if (device_is_running(dev) && strcmp(dev->name, name) == 0)
			return dev;
	}

	return NULL;
}

struct device *
dm_get_subdev_by_index(uint32_t class, uint8_t index,
                       uint8_t *id)
{
	for_each_dev_in_class(dev, class) {
		uint8_t start = dev->subdev_index;
		uint8_t end   = dev->subdev_index + dev->subdev_count;

		if (index >= start && index < end) {
			*id = index - start;
			return dev;
		}
	}

	return NULL;
}

struct device *
dm_next_subdev(struct device *dev, uint8_t *id)
{
	/* Case when the given subdevice is not the last in its controller. */
	if (++*id < dev->subdev_count)
		return dev;

	/* Otherwise, it is the first subdevice in the next device. */
	*id = 0;

	return dm_next_dev_by_class(dev->drv->class, dev);
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
