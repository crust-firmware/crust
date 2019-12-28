/*
 * Copyright © 2017-2019 The Crust Firmware Authors.
 * SPDX-License-Identifier: BSD-3-Clause OR GPL-2.0-only
 */

#include <device.h>
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

static uint8_t refcount;

int
axp803_get(const struct rsb_handle *bus)
{
	uint8_t reg;
	int err;

	if (refcount)
		return SUCCESS;
	if ((err = rsb_get(bus, AXP803_RSB_HWADDR,
	                   AXP803_MODE_REG, AXP803_MODE_VAL)))
		return err;
	if ((err = rsb_read(bus, IC_TYPE_REG, &reg)))
		goto err_put_bus;
	if ((reg & IC_TYPE_MASK) != IC_TYPE_VALUE) {
		err = ENODEV;
		goto err_put_bus;
	}

	++refcount;

	return SUCCESS;

err_put_bus:
	axp803_put(bus);
	return err;
}

void
axp803_put(const struct rsb_handle *bus)
{
	if (--refcount)
		return;

	rsb_put(bus);
}

int
axp803_reg_setbits(const struct rsb_handle *bus, uint8_t addr, uint8_t bits)
{
	uint8_t tmp;
	int err;

	if ((err = rsb_read(bus, addr, &tmp)))
		return err;

	return rsb_write(bus, addr, tmp | bits);
}
