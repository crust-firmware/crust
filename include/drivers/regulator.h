/*
 * Copyright © 2017-2020 The Crust Firmware Authors.
 * SPDX-License-Identifier: BSD-3-Clause OR GPL-2.0-only
 */

#ifndef DRIVERS_REGULATOR_H
#define DRIVERS_REGULATOR_H

#include <device.h>
#include <stdint.h>

struct regulator_list {
	const struct device *dev;    /**< The regulator supplier device. */
	const uint8_t       *ids;    /**< A list of regulator identifiers. */
	uint8_t              nr_ids; /**< The number of identifiers given. */
};

/**
 * Disable the output of one or more regulators. If a regulator does not have
 * output on/off control, this function may have no effect on the hardware.
 *
 * This function will acquire and release a reference to the supplier device.
 *
 * This function will return the first error code encountered, but it will
 * attempt to disable the remaining regulators in the list.
 *
 * This function may fail with:
 *   EIO    There was a problem communicating with the hardware.
 *
 * @param list  A list of one or more regulators
 * @return      Zero on success; a defined error code on failure.
 */
int regulator_bulk_disable(const struct regulator_list *list);

/**
 * Enable the output of one or more regulators. If a regulator does not have
 * output on/off control, this function may have no effect on the hardware.
 *
 * This function will acquire and release a reference to the supplier device.
 *
 * This function will return the first error code encountered, but it will
 * attempt to enable the remaining regulators in the list.
 *
 * This function may fail with:
 *   EIO    There was a problem communicating with the hardware.
 *
 * @param list  A list of one or more regulators
 * @return      Zero on success; a defined error code on failure.
 */
int regulator_bulk_enable(const struct regulator_list *list);

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
