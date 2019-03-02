/*
 * Copyright © 2017-2019 The Crust Firmware Authors.
 * SPDX-License-Identifier: BSD-3-Clause OR GPL-2.0-only
 */

#include <dm.h>
#include <error.h>
#include <rsb.h>
#include <stdbool.h>
#include <stdint.h>
#include <mfd/axp803.h>

#define AXP803_MODE_REG 0x3e
#define AXP803_MODE_VAL 0x7c

#define IC_TYPE_REG     0x03
#define IC_TYPE_MASK    0xcf
#define IC_TYPE_VALUE   0x41

static bool initialized;

int
axp803_init_once(struct device *dev)
{
	struct device *bus = dev->bus;
	uint32_t addr      = RSB_RTADDR(dev->addr) | AXP803_RSB_HWADDR;
	uint8_t  reg;
	int err;

	if (initialized)
		return SUCCESS;
	if ((err = rsb_init_pmic(bus, addr, AXP803_MODE_REG, AXP803_MODE_VAL)))
		return err;
	if ((err = rsb_read(bus, dev->addr, IC_TYPE_REG, &reg)))
		return err;
	if ((reg & IC_TYPE_MASK) != IC_TYPE_VALUE)
		return ENODEV;

	initialized = true;

	return SUCCESS;
}

int
axp803_reg_setbits(struct device *dev, uint8_t reg, uint8_t bits)
{
	struct device *bus = dev->bus;
	uint8_t addr       = dev->addr;
	uint8_t tmp;
	int err;

	if ((err = rsb_read(bus, addr, reg, &tmp)))
		return err;

	return rsb_write(bus, addr, reg, tmp | bits);
}
