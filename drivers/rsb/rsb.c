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
rsb_probe(const struct rsb_handle *bus, uint16_t hwaddr, uint8_t addr,
          uint8_t data)
{
	/* Ensure the controller's driver is loaded. */
	if (!device_get(bus->dev))
		return ENODEV;

	return rsb_ops_for(bus)->probe(bus, hwaddr, addr, data);
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
