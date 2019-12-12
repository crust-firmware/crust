/*
 * Copyright © 2017-2019 The Crust Firmware Authors.
 * SPDX-License-Identifier: BSD-3-Clause OR GPL-2.0-only
 */

#include <debug.h>
#include <dm.h>
#include <error.h>
#include <intrusive.h>
#include <pmic.h>
#include <rsb.h>
#include <system_power.h>
#include <mfd/axp803.h>
#include <pmic/axp803.h>
#include <rsb/sunxi-rsb.h>

#define WAKEUP_CTRL_REG   0x31
#define POWER_DISABLE_REG 0x32
#define PIN_FUNCTION_REG  0x8f

static inline struct axp803_pmic *
to_axp803_pmic(struct device *dev)
{
	return container_of(dev, struct axp803_pmic, dev);
}

static int
axp803_pmic_reset(struct device *dev)
{
	struct axp803_pmic *self = to_axp803_pmic(dev);

	/* Trigger soft power restart. */
	return axp803_reg_setbits(&self->bus, WAKEUP_CTRL_REG, BIT(6));
}

static int
axp803_pmic_resume(struct device *dev)
{
	struct axp803_pmic *self = to_axp803_pmic(dev);

	/* Trigger soft power resume. */
	return axp803_reg_setbits(&self->bus, WAKEUP_CTRL_REG, BIT(5));
}

static int
axp803_pmic_shutdown(struct device *dev)
{
	struct axp803_pmic *self = to_axp803_pmic(dev);

	/* Trigger soft power off. */
	return axp803_reg_setbits(&self->bus, POWER_DISABLE_REG, BIT(7));
}

static int
axp803_pmic_suspend(struct device *dev)
{
	struct axp803_pmic *self = to_axp803_pmic(dev);

	/* Enable resume, allow IRQs during suspend. */
	return axp803_reg_setbits(&self->bus, WAKEUP_CTRL_REG,
	                          BIT(4) | BIT(3));
}

static int
axp803_pmic_probe(struct device *dev)
{
	struct axp803_pmic *self = to_axp803_pmic(dev);
	int err;

	if ((err = axp803_probe(&self->bus)))
		return err;

	/* Enable shutdown on PMIC overheat or >16 seconds button press;
	 * remember previous voltages when waking up from suspend. */
	if ((err = axp803_reg_setbits(&self->bus, PIN_FUNCTION_REG,
	                              GENMASK(3, 1))))
		return err;

	return SUCCESS;
}

static const struct pmic_driver axp803_pmic_driver = {
	.drv = {
		.probe = axp803_pmic_probe,
	},
	.ops = {
		.reset    = axp803_pmic_reset,
		.resume   = axp803_pmic_resume,
		.shutdown = axp803_pmic_shutdown,
		.suspend  = axp803_pmic_suspend,
	},
};

struct axp803_pmic axp803_pmic = {
	.dev = {
		.name = "axp803-pmic",
		.drv  = &axp803_pmic_driver.drv,
	},
	.bus = {
		.dev  = &r_rsb.dev,
		.addr = AXP803_RSB_RTADDR,
	},
};
