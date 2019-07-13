/*
 * Copyright © 2017-2019 The Crust Firmware Authors.
 * SPDX-License-Identifier: BSD-3-Clause OR GPL-2.0-only
 */

#include <clock.h>
#include <debug.h>
#include <dm.h>
#include <error.h>
#include <gpio.h>
#include <irqchip.h>
#include <stdbool.h>
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

	/* Tell the device the index of its first subdevice. */
	dev->subdev_index = total_subdevs[dev->drv->class];

	/* Probe the device itself, and report any errors. */
	if ((err = dev->drv->probe(dev)))
		panic("dm: Failed to probe %s (%d)", dev->name, err);

	dev->flags |= DEVICE_FLAG_RUNNING;
	/* If the driver's probe function did not provide the number of
	 * subdevices, assume the default number of 1. */
	if (dev->subdev_count == 0)
		dev->subdev_count = 1;
	/* Update the total number of subdevices for this class. */
	total_subdevs[dev->drv->class] += dev->subdev_count;

	debug("dm: Probed %s", dev->name);
}

uint8_t
dm_count_subdevs_by_class(uint32_t class)
{
	return total_subdevs[class];
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
	int8_t start = handle->index + 1;

	for (dev = &device_list[start]; dev < device_list_end; ++dev) {
		if (!device_is_running(dev))
			continue;
		if (dev->drv->class == handle->class) {
			handle->dev   = dev;
			handle->index = dev - device_list;
			return SUCCESS;
		}
	}

	return ENODEV;
}

int
dm_get_subdev_by_index(struct device_handle *handle, uint8_t class,
                       uint8_t index)
{
	*handle = DEVICE_HANDLE_INIT(class);

	while (dm_next_dev_by_class(handle) == SUCCESS) {
		uint8_t start = handle->dev->subdev_index;
		uint8_t end   = start + handle->dev->subdev_count;

		if (index >= start && index < end) {
			handle->id = index - start;
			return SUCCESS;
		}
	}

	return ENODEV;
}

int
dm_next_subdev(struct device_handle *handle)
{
	/* Case when the given subdevice is not the last in its controller. */
	if (handle->dev && ++handle->id < handle->dev->subdev_count)
		return SUCCESS;

	/* Otherwise, it is the first subdevice in the next device. */
	handle->id = 0;

	return dm_next_dev_by_class(handle);
}

void
dm_init(void)
{
	for (struct device *dev = device_list; dev < device_list_end; ++dev)
		device_probe(dev);
}

void
dm_poll(void)
{
	for (struct device *dev = device_list; dev < device_list_end; ++dev) {
		void (*poll)(struct device *) = dev->drv->poll;
		if (poll)
			poll(dev);
	}
}

int
dm_setup_clocks(struct device *dev, uint8_t num_clocks)
{
	struct clock_handle *clocks = dev->clocks;
	int err;

	for (uint8_t i = 0; i < num_clocks; ++i) {
		struct device *clockdev = clocks[i].dev;
		uint8_t id = clocks[i].id;

		/* Probe to ensure clock controller's driver is loaded. */
		device_probe(clockdev);

		/* Enable each clock used by the device. */
		if ((err = clock_enable(clockdev, id)))
			return err;
	}

	return SUCCESS;
}

int
dm_setup_irq(struct device *dev, bool (*fn)(struct device *))
{
	struct device *irqchip;
	struct irq_handle *handle = dev->irq;

	/* Replace the irqchip reference with a link to the child device. */
	irqchip     = handle->dev;
	handle->dev = dev;
	/* Put the function reference in the handle. */
	handle->fn = fn;

	/* Probe to ensure the interrupt controller's driver is loaded. */
	device_probe(irqchip);

	return irqchip_enable(irqchip, handle);
}

int
dm_setup_pins(struct device *dev, uint8_t num_pins)
{
	struct gpio_handle *pins = dev->pins;
	int err;

	for (uint8_t i = 0; i < num_pins; ++i) {
		struct device *gpiodev = pins[i].dev;
		uint8_t id   = pins[i].pin;
		uint8_t mode = pins[i].mode;

		/* Probe to ensure GPIO controller's driver is loaded. */
		device_probe(gpiodev);

		/* Set pin mode and return if error occurs. */
		if ((err = gpio_set_mode(gpiodev, id, mode)))
			return err;
	}

	return SUCCESS;
}
