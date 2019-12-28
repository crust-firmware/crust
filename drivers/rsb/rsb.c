/*
 * Copyright © 2017-2019 The Crust Firmware Authors.
 * SPDX-License-Identifier: BSD-3-Clause OR GPL-2.0-only
 */

#include <device.h>
#include <error.h>
#include <intrusive.h>
#include <rsb.h>
#include <stdint.h>

#include "rsb.h"

/**
 * Get the ops for the RSB controller device.
 */
static inline const struct rsb_driver_ops *
rsb_ops_for(const struct rsb_handle *bus)
{
	const struct rsb_driver *drv =
		container_of(bus->dev->drv, const struct rsb_driver, drv);

	return &drv->ops;
}

int
rsb_get(const struct rsb_handle *bus, uint16_t hwaddr, uint8_t addr,
        uint8_t data)
{
	int err;

	/* Ensure the controller's driver is loaded. */
	if (!device_get(bus->dev))
		return ENODEV;

	if ((err = rsb_ops_for(bus)->probe(bus, hwaddr, addr, data)))
		goto err_put_device;

	return SUCCESS;

err_put_device:
	device_put(bus->dev);

	return err;
}

void
rsb_put(const struct rsb_handle *bus)
{
	device_put(bus->dev);
}

int
rsb_read(const struct rsb_handle *bus, uint8_t addr, uint8_t *data)
{
	return rsb_ops_for(bus)->read(bus, addr, data);
}

int
rsb_set_rate(const struct rsb_handle *bus, uint32_t rate)
{
	return rsb_ops_for(bus)->set_rate(bus->dev, rate);
}

int
rsb_write(const struct rsb_handle *bus, uint8_t addr, uint8_t data)
{
	return rsb_ops_for(bus)->write(bus, addr, data);
}
