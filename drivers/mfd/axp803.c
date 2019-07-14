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
axp803_probe(struct rsb_handle *bus)
{
	uint8_t reg;
	int err;

	if (initialized)
		return SUCCESS;
	if ((err = rsb_probe(bus, AXP803_RSB_HWADDR,
	                     AXP803_MODE_REG, AXP803_MODE_VAL)))
		return err;
	if ((err = rsb_read(bus, IC_TYPE_REG, &reg)))
		return err;
	if ((reg & IC_TYPE_MASK) != IC_TYPE_VALUE)
		return ENODEV;

	initialized = true;

	return SUCCESS;
}

int
axp803_reg_setbits(struct rsb_handle *bus, uint8_t addr, uint8_t bits)
{
	uint8_t tmp;
	int err;

	if ((err = rsb_read(bus, addr, &tmp)))
		return err;

	return rsb_write(bus, addr, tmp | bits);
}
