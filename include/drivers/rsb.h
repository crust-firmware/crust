/*
 * Copyright © 2017-2019 The Crust Firmware Authors.
 * SPDX-License-Identifier: BSD-3-Clause OR GPL-2.0-only
 */

#ifndef DRIVERS_RSB_H
#define DRIVERS_RSB_H

#include <device.h>
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
	int (*probe)(struct rsb_handle *bus, uint16_t hwaddr, uint8_t addr,
	             uint8_t data);
	int (*read)(struct rsb_handle *bus, uint8_t addr, uint8_t *data);
	int (*set_rate)(struct device *dev, uint32_t rate);
	int (*write)(struct rsb_handle *bus, uint8_t addr, uint8_t data);
};

struct rsb_driver {
	const struct driver         drv;
	const struct rsb_driver_ops ops;
};

/**
 * Probe for an RSB device, switch it to RSB mode, and set its runtime address.
 *
 * @param bus    The RSB bus that the device is connected to.
 * @param hwaddr The hardware address of this device.
 * @param addr   The register used to switch the PMIC to RSB mode.
 * @param data   The data value that will switch the PMIC to RSB mode.
 */
int rsb_probe(struct rsb_handle *bus, uint16_t hwaddr, uint8_t addr,
              uint8_t data);

/**
 * Read a register contained inside an RSB device.
 *
 * @param bus   The RSB bus that the device is connected to.
 * @param addr  The register within the the RSB device to read.
 * @param data  The location to save the data read from the register.
 */
static inline int
rsb_read(struct rsb_handle *bus, uint8_t addr, uint8_t *data)
{
	return RSB_OPS(bus->dev)->read(bus, addr, data);
}

static inline int
rsb_set_rate(struct rsb_handle *bus, uint32_t rate)
{
	return RSB_OPS(bus->dev)->set_rate(bus->dev, rate);
}

/**
 * Write to a register contained inside an RSB device.
 *
 * @param bus   The RSB bus that the device is connected to.
 * @param addr  The register within the the RSB device to write.
 * @param data  The data to write to the register.
 */
static inline int
rsb_write(struct rsb_handle *bus, uint8_t addr, uint8_t data)
{
	return RSB_OPS(bus->dev)->write(bus, addr, data);
}

#endif /* DRIVERS_RSB_H */
