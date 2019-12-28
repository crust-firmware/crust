/*
 * Copyright © 2017-2019 The Crust Firmware Authors.
 * SPDX-License-Identifier: BSD-3-Clause OR GPL-2.0-only
 */

#ifndef RSB_PRIVATE_H
#define RSB_PRIVATE_H

#include <device.h>
#include <rsb.h>
#include <stdint.h>

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

struct rsb_driver_ops {
	int (*probe)(const struct rsb_handle *bus, uint16_t hwaddr,
	             uint8_t addr, uint8_t data);
	int (*read)(const struct rsb_handle *bus, uint8_t addr, uint8_t *data);
	int (*set_rate)(const struct device *dev, uint32_t rate);
	int (*write)(const struct rsb_handle *bus, uint8_t addr, uint8_t data);
};

struct rsb_driver {
	struct driver         drv;
	struct rsb_driver_ops ops;
};

#endif /* RSB_PRIVATE_H */
