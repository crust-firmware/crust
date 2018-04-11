/*
 * Copyright © 2017-2018 The Crust Firmware Authors.
 * SPDX-License-Identifier: (BSD-3-Clause OR GPL-2.0)
 */

#include <debug.h>
#include <dm.h>
#include <error.h>
#include <i2c.h>
#include <pmic.h>
#include <mfd/axp803.h>
#include <pmic/axp803.h>

#define IRQ_ENABLE_REG1   0x40
#define IRQ_ENABLE_REG6   0x45
#define IRQ_STATUS_REG1   0x48
#define IRQ_STATUS_REG6   0x4d

#define WAKEUP_CTRL_REG   0x31
#define POWER_DISABLE_REG 0x32
#define POK_CTRL_REG      0x59
#define PIN_FUNCTION_REG  0x8f

static int
axp803_pmic_reset(struct device *dev)
{
	/* Trigger soft power restart. */
	return axp803_reg_setbits(dev, WAKEUP_CTRL_REG, BIT(6));
}

static int
axp803_pmic_shutdown(struct device *dev)
{
	/* Trigger soft power off. */
	return axp803_reg_setbits(dev, POWER_DISABLE_REG, BIT(7));
}

static int
axp803_pmic_suspend(struct device *dev)
{
	/* Enable wakeup, allow IRQs during suspend. */
	return axp803_reg_setbits(dev, WAKEUP_CTRL_REG, BIT(4) | BIT(3));
}

static int
axp803_pmic_wakeup(struct device *dev)
{
	/* Trigger soft power wakeup. */
	return axp803_reg_setbits(dev, WAKEUP_CTRL_REG, BIT(5));
}

static int
axp803_pmic_probe(struct device *dev)
{
	struct device *bus = dev->bus;
	int     err;
	uint8_t addr = dev->addr;

	if ((err = i2c_probe(bus, dev->addr)))
		return err;
	if ((err = axp803_match_type(dev)))
		return err;

	/* Set up the power button. */
	if ((err = i2c_write_reg(bus, addr, POK_CTRL_REG, BIT(4))))
		return err;

	/* Disable all unwanted IRQs. */
	for (uint8_t reg = IRQ_ENABLE_REG1; reg <= IRQ_ENABLE_REG6; ++reg) {
		if ((err = i2c_write_reg(bus, addr, reg, 0)))
			return err;
	}

	/* Now clear all IRQs. */
	for (uint8_t reg = IRQ_STATUS_REG1; reg <= IRQ_STATUS_REG6; ++reg) {
		if ((err = i2c_write_reg(bus, addr, reg, BITMASK(0, 8))))
			return err;
	}

	/* Enable shutdown on PMIC overheat or >16 seconds button press;
	 * remember previous voltages when waking up from suspend. */
	if ((err = axp803_reg_setbits(dev, PIN_FUNCTION_REG, BITMASK(1, 3))))
		return err;

	return SUCCESS;
}

const struct pmic_driver axp803_pmic_driver = {
	.drv = {
		.name  = "axp803-pmic",
		.class = DM_CLASS_PMIC,
		.probe = axp803_pmic_probe,
	},
	.ops = {
		.reset    = axp803_pmic_reset,
		.shutdown = axp803_pmic_shutdown,
		.suspend  = axp803_pmic_suspend,
		.wakeup   = axp803_pmic_wakeup,
	},
};
