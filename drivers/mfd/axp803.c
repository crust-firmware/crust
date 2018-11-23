/*
 * Copyright © 2017-2018 The Crust Firmware Authors.
 * SPDX-License-Identifier: BSD-3-Clause OR GPL-2.0-only
 */

#include <dm.h>
#include <error.h>
#include <i2c.h>
#include <stdbool.h>
#include <stdint.h>

#define IC_TYPE_REG   0x03
#define IC_TYPE_MASK  0xcf
#define IC_TYPE_VALUE 0x41

int
axp803_match_type(struct device *dev)
{
	int     err;
	uint8_t reg;

	if ((err = i2c_read_reg(dev->bus, dev->addr, IC_TYPE_REG, &reg)))
		return err;
	if ((reg & IC_TYPE_MASK) != IC_TYPE_VALUE)
		return ENODEV;

	return SUCCESS;
}

int
axp803_reg_setbits(struct device *dev, uint8_t reg, uint8_t bits)
{
	struct device *bus = dev->bus;
	int     err;
	uint8_t addr = dev->addr;
	uint8_t tmp;

	if ((err = i2c_read_reg(bus, addr, reg, &tmp)))
		return err;

	return i2c_write_reg(bus, addr, reg, tmp | bits);
}
