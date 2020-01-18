/*
 * Copyright © 2017-2020 The Crust Firmware Authors.
 * SPDX-License-Identifier: BSD-3-Clause OR GPL-2.0-only
 */

#ifndef DRIVERS_REGULATOR_H
#define DRIVERS_REGULATOR_H

#include <device.h>
#include <stdint.h>

struct regulator_handle {
	const struct device *dev; /**< The device containing this regulator. */
	uint8_t              id;  /**< The per-device regulator identifier. */
};

/**
 * Disable the output of a regulator. If the regulator does not have output
 * on/off control, this function may have no effect on the hardware.
 *
 * This function may fail with:
 *   EIO    There was a problem communicating with the hardware.
 *   EPERM  The regulator is critical and cannot be disabled.
 *
 * @param dev   The device containing this regulator.
 * @param id    The device-specific identifier for this regulator.
 * @return      Zero on success; a defined error code on failure.
 */
int regulator_disable(const struct device *dev, uint8_t id);

/**
 * Enable the output of a regulator. If the regulator does not have output
 * on/off control, this function may have no effect on the hardware.
 *
 * This function may fail with:
 *   EIO    There was a problem communicating with the hardware.
 *
 * @param dev   The device containing this regulator.
 * @param id    The device-specific identifier for this regulator.
 * @return      Zero on success; a defined error code on failure.
 */
int regulator_enable(const struct device *dev, uint8_t id);

/**
 * Get the current state of a regulator, as determined from the hardware.
 *
 * This function may fail with:
 *   EIO    There was a problem communicating with the hardware.
 *
 * @param dev   The device containing this regulator.
 * @param id    The device-specific identifier for this regulator.
 * @return      On success, boolean true or false for if the regulator is
 *              enabled; a defined error code on failure.
 */
int regulator_get_state(const struct device *dev, uint8_t id);

#endif /* DRIVERS_REGULATOR_H */
