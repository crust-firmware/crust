/*
 * Copyright © 2017-2020 The Crust Firmware Authors.
 * SPDX-License-Identifier: BSD-3-Clause OR GPL-2.0-only
 */

#ifndef DRIVERS_REGMAP_H
#define DRIVERS_REGMAP_H

#include <device.h>
#include <stdint.h>

struct regmap {
	const struct device *dev;
	uint8_t              id;
};

/**
 * Get a reference to a regmap and its provider bus/device.
 *
 * This function will verify the existence of the device containing the regmap.
 *
 * This function may fail with:
 *   EIO     There was a problem communicating with the hardware.
 *   ENODEV  The device was not found on its provider bus.
 *
 * @param map  A specifier for the bus/device providing the regmap.
 * @return     Zero on success; an error code on failure.
 */
int regmap_get(const struct regmap *map);

/**
 * Release a reference to a regmap.
 *
 * @param map  A reference to a regmap.
 */
void regmap_put(const struct regmap *map);

/**
 * Read a value from a regmap.
 *
 * This function may fail with:
 *   EIO     There was a problem communicating with the hardware.
 *
 * @param map  A reference to the regmap.
 * @param reg  The register to read.
 * @param val  The location to save the value read from the register.
 * @return     Zero on success; an error code on failure.
 */
int regmap_read(const struct regmap *map, uint8_t reg, uint8_t *val);

/**
 * Set bits in a regmap register.
 *
 * This function may fail with:
 *   EIO     There was a problem communicating with the hardware.
 *
 * @param map  A reference to the regmap.
 * @param reg  The register to modify.
 * @param set  The mask of bits to set in the register.
 * @return     Zero on success; an error code on failure.
 */
int regmap_set_bits(const struct regmap *map, uint8_t reg, uint8_t set);

/**
 * Update a bitfield in a regmap register.
 *
 * This function may fail with:
 *   EIO     There was a problem communicating with the hardware.
 *
 * @param map  A reference to the regmap.
 * @param reg  The register to modify.
 * @param mask A mask of bits representing a bitfield.
 * @param val  The new value of the bitfield.
 * @return     Zero on success; an error code on failure.
 */
int regmap_update_bits(const struct regmap *map, uint8_t reg, uint8_t mask,
                       uint8_t val);

/**
 * Write a value to a regmap.
 *
 * This function may fail with:
 *   EIO     There was a problem communicating with the hardware.
 *
 * @param map  A reference to the regmap.
 * @param reg  The register to write.
 * @param val  The value to write to the register.
 * @return     Zero on success; an error code on failure.
 */
int regmap_write(const struct regmap *map, uint8_t reg, uint8_t val);

#endif /* DRIVERS_REGMAP_H */
