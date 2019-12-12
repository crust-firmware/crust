/*
 * Copyright © 2017-2019 The Crust Firmware Authors.
 * SPDX-License-Identifier: BSD-3-Clause OR GPL-2.0-only
 */

#include <device.h>
#include <rsb.h>

int
rsb_probe(struct rsb_handle *bus, uint16_t hwaddr, uint8_t addr, uint8_t data)
{
	/* Ensure the controller's driver is loaded. */
	device_probe(bus->dev);

	return RSB_OPS(bus->dev)->probe(bus, hwaddr, addr, data);
}
