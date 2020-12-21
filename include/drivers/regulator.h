/*
 * Copyright © 2017-2020 The Crust Firmware Authors.
 * SPDX-License-Identifier: BSD-3-Clause OR GPL-2.0-only
 */

#ifndef DRIVERS_REGULATOR_H
#define DRIVERS_REGULATOR_H

#include <device.h>
#include <stdint.h>

struct regulator_handle {
	const struct device *dev; /**< The regulator supplier device. */
	uint8_t              id;  /**< The device-specific identifier. */
};

/**
 * Disable the output of a regulator. If the regulator does not have
 * output on/off control, this function may have no effect on the hardware.
 *
 * This function will acquire and release a reference to the supplier device.
 *
 * This function may fail with:
 *   EIO    There was a problem communicating with the hardware.
 *
 * @param handle A reference to a regulator and its supplier.
 * @return       Zero on success; a defined error code on failure.
 */
int regulator_disable(const struct regulator_handle *handle);

/**
 * Enable the output of a regulator. If the regulator does not have
 * output on/off control, this function may have no effect on the hardware.
 *
 * This function will acquire and release a reference to the supplier device.
 *
 * This function may fail with:
 *   EIO    There was a problem communicating with the hardware.
 *
 * @param handle A reference to a regulator and its supplier.
 * @return       Zero on success; a defined error code on failure.
 */
int regulator_enable(const struct regulator_handle *handle);

/**
 * Get the current state of a regulator, as determined from the hardware.
 *
 * This function may fail with:
 *   EIO    There was a problem communicating with the hardware.
 *
 * @param handle A reference to a regulator and its supplier.
 * @return       On success, Boolean true or false for if the regulator is
 *               enabled; a defined error code on failure.
 */
int regulator_get_state(const struct regulator_handle *handle);

#endif /* DRIVERS_REGULATOR_H */
