/*
 * Copyright © 2017-2018 The Crust Firmware Authors.
 * SPDX-License-Identifier: BSD-3-Clause OR GPL-2.0-only
 */

#ifndef DRIVERS_DVFS_H
#define DRIVERS_DVFS_H

#include <dm.h>
#include <intrusive.h>
#include <stdbool.h>
#include <stdint.h>
#include <util.h>

#define DVFS_OPS(dev) \
	(&container_of((dev)->drv, struct dvfs_driver, drv)->ops)

struct dvfs_opp {
	const uint16_t rate;    /**< Clock rate in MHz. */
	const uint16_t voltage; /**< Supply voltage in mV. */
};

struct dvfs_info {
	const uint16_t         latency;   /**< Worst switching time in μs. */
	const uint8_t          opp_count; /**< Number of operating points. */
	const struct dvfs_opp *opp_table; /**< Table of operating points. */
};

struct dvfs_driver_ops {
	struct dvfs_info *(*get_info)(struct device *dev, uint8_t id);
	uint8_t           (*get_opp)(struct device *dev, uint8_t id);
	int               (*set_opp)(struct device *dev, uint8_t id,
	                             uint8_t opp);
};

struct dvfs_driver {
	const struct driver          drv;
	const struct dvfs_driver_ops ops;
};

/**
 * Get generic information about a DVFS domain.
 *
 * This function has no defined errors.
 *
 * @param dev The DVFS controller containing this domain.
 * @param id  The device-specific identifier for this domain.
 * @return    A pointer to the information structure.
 */
static inline struct dvfs_info *
dvfs_get_info(struct device *dev, uint8_t id)
{
	return DVFS_OPS(dev)->get_info(dev, id);
}

/**
 * Get the current power level of a DVFS domain, as an index into its operating
 * point table.
 *
 * This function has no defined errors.
 *
 * @param dev The DVFS controller containing this domain.
 * @param id  The device-specific identifier for this domain.
 * @return    Zero on success; a defined error code on failure.
 */
static inline uint8_t
dvfs_get_opp(struct device *dev, uint8_t id)
{
	return DVFS_OPS(dev)->get_opp(dev, id);
}

/**
 * Set the current power level of a DVFS domain, given an index into its
 * operating point table.
 *
 * This function may fail with:
 *   EIO    There was a problem communicating with the hardware.
 *   ERANGE The requested index is beyond the end of the operating point table.
 *
 * @param dev The DVFS controller containing this domain.
 * @param id  The device-specific identifier for this domain.
 * @param opp The location to store the operating point index.
 */
static inline int
dvfs_set_opp(struct device *dev, uint8_t id, uint8_t opp)
{
	return DVFS_OPS(dev)->set_opp(dev, id, opp);
}

#endif /* DRIVERS_DVFS_H */
