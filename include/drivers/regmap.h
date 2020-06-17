/*
 * Copyright © 2017-2020 The Crust Firmware Authors.
 * SPDX-License-Identifier: BSD-3-Clause OR GPL-2.0-only
 */

#ifndef DRIVERS_REGMAP_H
#define DRIVERS_REGMAP_H

#include <device.h>
#include <intrusive.h>
#include <stdint.h>

struct regmap {
	const struct device *dev;
	uint8_t              id;
};

struct regmap_device {
	struct device dev;
	struct regmap map;
};

/**
 * Downcast a pointer to a regmap device.
 */
static inline const struct regmap_device *
to_regmap_device(const struct device *dev)
{
	return container_of(dev, const struct regmap_device, dev);
}

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
 * Clear bits in a regmap register.
 *
 * This function may fail with:
 *   EIO     There was a problem communicating with the hardware.
 *
 * @param map  A reference to the regmap.
 * @param reg  The register to modify.
 * @param clr  The mask of bits to clear in the register.
 * @return     Zero on success; an error code on failure.
 */
static inline int
regmap_clr_bits(const struct regmap *map, uint8_t reg, uint8_t clr)
{
	return regmap_update_bits(map, reg, clr, 0);
}

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
static inline int
regmap_set_bits(const struct regmap *map, uint8_t reg, uint8_t set)
{
	return regmap_update_bits(map, reg, set, set);
}

/**
 * Probe a device that owns a regmap.
 *
 * This function can be used to implement a device's .probe hook.
 */
int regmap_device_probe(const struct device *dev);

/**
 * Release a device that owns a regmap.
 *
 * This function can be used to implement a device's .release hook.
 */
void regmap_device_release(const struct device *dev);

/**
 * Probe a device that uses a regmap owned by another device.
 *
 * This function assumes that the regmap's owner is a regmap_device.
 *
 * This function can be used to implement a device's .probe hook.
 */
int regmap_user_probe(const struct regmap *map);

/**
 * Release a device that uses a regmap owned by another device.
 *
 * This function assumes that the regmap's owner is a regmap_device.
 *
 * This function can be used to implement a device's .release hook.
 */
void regmap_user_release(const struct regmap *map);

#endif /* DRIVERS_REGMAP_H */
