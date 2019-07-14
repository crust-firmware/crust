/*
 * Copyright © 2017-2019 The Crust Firmware Authors.
 * SPDX-License-Identifier: BSD-3-Clause OR GPL-2.0-only
 */

#ifndef DRIVERS_RSB_H
#define DRIVERS_RSB_H

#include <dm.h>
#include <intrusive.h>
#include <stdint.h>

#define RSB_OPS(dev) \
	(&container_of((dev)->drv, struct rsb_driver, drv)->ops)

#define RSB_NUM_PINS     2

#define RSB_RTADDR(addr) ((addr) << 16)

enum {
	RSB_SRTA = 0xe8,
	RSB_RD8  = 0x8b,
	RSB_RD16 = 0x9c,
	RSB_RD32 = 0xa6,
	RSB_WR8  = 0x4e,
	RSB_WR16 = 0x59,
	RSB_WR32 = 0x63,
};

struct rsb_handle {
	struct device *dev;
	uint8_t        addr;
};

struct rsb_driver_ops {
	int (*init_pmic)(struct device *dev, uint32_t addr, uint8_t reg,
	                 uint8_t data);
	int (*read)(struct device *dev, uint8_t addr, uint8_t reg,
	            uint8_t *data);
	int (*set_rate)(struct device *dev, uint32_t rate);
	int (*write)(struct device *dev, uint8_t addr, uint8_t reg,
	             uint8_t data);
};

struct rsb_driver {
	const struct driver         drv;
	const struct rsb_driver_ops ops;
};

/**
 * Probe for an RSB PMIC, switch it to RSB mode, and set its runtime address.
 *
 * @param dev   The RSB controller that the device is connected to.
 * @param addr  A pointer to a word containing the device address in the two
 *              least significant bytes, and where the runtime address will be
 *              stored in the third byte.
 * @param reg   The register used to switch the PMIC to RSB mode.
 * @param data  The data value that will switch the PMIC to RSB mode.
 */
static inline int
rsb_init_pmic(struct device *dev, uint32_t addr, uint8_t reg, uint8_t data)
{
	return RSB_OPS(dev)->init_pmic(dev, addr, reg, data);
}

/**
 * Read a register contained inside an RSB device.
 *
 * @param dev   The RSB controller that the device is connected to.
 * @param addr  The address of the RSB device (as passed to rsb_probe).
 * @param reg   The register within the the RSB device to read.
 * @param data  The location to save the data read from the register.
 */
static inline int
rsb_read(struct device *dev, uint8_t addr, uint8_t reg, uint8_t *data)
{
	return RSB_OPS(dev)->read(dev, addr, reg, data);
}

static inline int
rsb_set_rate(struct device *dev, uint32_t rate)
{
	return RSB_OPS(dev)->set_rate(dev, rate);
}

/**
 * Write to a register contained inside an RSB device.
 *
 * @param dev   The RSB controller that the device is connected to.
 * @param addr  The address of the RSB device (as passed to rsb_probe).
 * @param reg   The register within the the RSB device to write.
 * @param data  The data to write to the register.
 */
static inline int
rsb_write(struct device *dev, uint8_t addr, uint8_t reg, uint8_t data)
{
	return RSB_OPS(dev)->write(dev, addr, reg, data);
}

#endif /* DRIVERS_RSB_H */
