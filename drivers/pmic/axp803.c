/*
 * Copyright © 2017-2018 The Crust Firmware Authors.
 * SPDX-License-Identifier: (BSD-3-Clause OR GPL-2.0)
 */

#include <debug.h>
#include <error.h>
#include <i2c.h>
#include <pmic.h>

#define GPIO0_CTL            0x90
#define GPIO1_CTL            0x92

#define IC_TYPE_REG          0x03
#define IC_TYPE_MASK         0xcf
#define IC_TYPE_VALUE        0x41

#define IRQ_ENABLE1          0x40
#define IRQ_ENABLE2          0x41
#define IRQ_ENABLE4          0x43
#define IRQ_ENABLE5          0x44
#define IRQ_PIN              0x8f
#define IRQ_STATUS5          0x4c

#define POWER_DISABLE        0x32
#define POWER_WAKEUP_CONTROL 0x31

static void
axp803_shutdown(struct device *dev)
{
	/* Soft power off. */
	i2c_write_reg(dev->bus, dev->addr, POWER_DISABLE, BIT(7));
}

static void
axp803_suspend(struct device *dev)
{
	/* Enable ACIN IRQs. */
	i2c_write_reg(dev->bus, dev->addr, IRQ_ENABLE1, BITMASK(5, 2));

	/* Enable POK long time active and negative edge IRQs. */
	i2c_write_reg(dev->bus, dev->addr, IRQ_ENABLE5, BIT(3) | BIT(5));

	/* Enable battery capacity IRQs. */
	i2c_write_reg(dev->bus, dev->addr, IRQ_ENABLE4, BITMASK(0, 2));

	/* Enable GPIO positive and negative edge trigger IRQs. */
	i2c_write_reg(dev->bus, dev->addr, IRQ_STATUS5, BITMASK(0, 2));
	i2c_write_reg(dev->bus, dev->addr, GPIO0_CTL, BITMASK(6, 2));
	i2c_write_reg(dev->bus, dev->addr, GPIO1_CTL, BITMASK(6, 2));

	/* Wake up output power. */
	i2c_write_reg(dev->bus, dev->addr, POWER_WAKEUP_CONTROL, BIT(5));

	/* Enable wakeup PMIC function. */
	i2c_write_reg(dev->bus, dev->addr, IRQ_PIN, BIT(7));

	/* Enable battery charged and charging IRQs. */
	i2c_write_reg(dev->bus, dev->addr, IRQ_ENABLE2, BITMASK(2, 2));
}

static int
axp803_probe(struct device *dev __unused)
{
	int     err;
	uint8_t reg;

	if ((err = i2c_probe(dev->bus, dev->addr)))
		return err;
	if ((err = i2c_read_reg(dev->bus, dev->addr, IC_TYPE_REG, &reg)))
		return err;
	if ((reg & IC_TYPE_MASK) != IC_TYPE_VALUE)
		return ENODEV;

	return SUCCESS;
}

const struct pmic_driver axp803_pmic_driver = {
	.drv = {
		.name  = "axp803-pmic",
		.class = DM_CLASS_PMIC,
		.probe = axp803_probe,
	},
	.ops = {
		.shutdown = axp803_shutdown,
		.suspend  = axp803_suspend,
	},
};
