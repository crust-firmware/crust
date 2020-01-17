/*
 * Copyright © 2017-2020 The Crust Firmware Authors.
 * SPDX-License-Identifier: BSD-3-Clause OR GPL-2.0-only
 */

#ifndef DRIVERS_RSB_H
#define DRIVERS_RSB_H

#include <device.h>
#include <stdint.h>

struct rsb_handle {
	const struct device *dev;
	uint8_t              id;
};

/**
 * Get a reference to an RSB device.
 *
 * This function will probe for the existence of the RSB device.
 *
 * @param bus   The RSB bus that the device is connected to.
 * @return      Zero on success; an error code on failure.
 */
int rsb_get(const struct rsb_handle *bus);

/**
 * Release a reference to an RSB device.
 *
 * @param bus A reference to an RSB device.
 */
void rsb_put(const struct rsb_handle *bus);

/**
 * Read a register contained inside an RSB device.
 *
 * @param bus   The RSB bus that the device is connected to.
 * @param addr  The register within the the RSB device to read.
 * @param data  The location to save the data read from the register.
 * @return      Zero on success; an error code on failure.
 */
int rsb_read(const struct rsb_handle *bus, uint8_t addr, uint8_t *data);

/**
 * Write to a register contained inside an RSB device.
 *
 * @param bus   The RSB bus that the device is connected to.
 * @param addr  The register within the the RSB device to write.
 * @param data  The data to write to the register.
 * @return      Zero on success; an error code on failure.
 */
int rsb_write(const struct rsb_handle *bus, uint8_t addr, uint8_t data);

#endif /* DRIVERS_RSB_H */
