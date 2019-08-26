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
#include <irq/sun4i-intc.h>
#include <mfd/axp803.h>
#include <pmic/axp803.h>
#include <rsb/sunxi-rsb.h>

#define IRQ_ENABLE_REG1   0x40
#define IRQ_ENABLE_REG5   0x44
#define IRQ_ENABLE_REG6   0x45
#define IRQ_STATUS_REG1   0x48
#define IRQ_STATUS_REG5   0x4c
#define IRQ_STATUS_REG6   0x4d

#define GPIO0IRQ          BIT(0) /**< GPIO0 input edge. */
#define GPIO1IRQ          BIT(1) /**< GPIO1 input edge. */
#define POKOIRQ           BIT(2) /**< Power key power-off duration. */
#define POKLIRQ           BIT(3) /**< Power key long duration. */
#define POKSIRQ           BIT(4) /**< Power key short duration. */
#define POKNIRQ           BIT(5) /**< Power key negative edge. */
#define POKPIRQ           BIT(6) /**< Power key positive edge. */
#define EVENTIRQ          BIT(7) /**< Event timeout. */
#define WANTED_IRQS       (POKLIRQ | POKPIRQ)

#define WAKEUP_CTRL_REG   0x31
#define POWER_DISABLE_REG 0x32
#define PIN_FUNCTION_REG  0x8f

static inline struct axp803_pmic *
to_axp803_pmic(struct device *dev)
{
	return container_of(dev, struct axp803_pmic, dev);
}

static bool
axp803_pmic_irq(const struct irq_handle *irq)
{
	struct axp803_pmic *this = container_of(irq, struct axp803_pmic, irq);
	const char *name         = this->dev.name;
	uint8_t reg;
	int err;

	/* IRQ register 5 is the only one with enabled IRQs. */
	if ((err = rsb_read(&this->bus, IRQ_STATUS_REG5, &reg)))
		panic("%s: Cannot read NMI status: %d", name, err);
	if ((err = rsb_write(&this->bus, IRQ_STATUS_REG5, reg)))
		panic("%s: Cannot clear NMI 0x%02x: %d", name, reg, err);

	/* Reset on long press, otherwise wake the system. */
	if (reg & POKLIRQ)
		system_reset();
	else
		system_wakeup();

	return true;
}

static int
axp803_pmic_reset(struct device *dev)
{
	struct axp803_pmic *this = to_axp803_pmic(dev);

	/* Trigger soft power restart. */
	return axp803_reg_setbits(&this->bus, WAKEUP_CTRL_REG, BIT(6));
}

static int
axp803_pmic_resume(struct device *dev)
{
	struct axp803_pmic *this = to_axp803_pmic(dev);

	/* Trigger soft power resume. */
	return axp803_reg_setbits(&this->bus, WAKEUP_CTRL_REG, BIT(5));
}

static int
axp803_pmic_shutdown(struct device *dev)
{
	struct axp803_pmic *this = to_axp803_pmic(dev);

	/* Trigger soft power off. */
	return axp803_reg_setbits(&this->bus, POWER_DISABLE_REG, BIT(7));
}

static int
axp803_pmic_suspend(struct device *dev)
{
	struct axp803_pmic *this = to_axp803_pmic(dev);

	/* Enable resume, allow IRQs during suspend. */
	return axp803_reg_setbits(&this->bus, WAKEUP_CTRL_REG,
	                          BIT(4) | BIT(3));
}

static int
axp803_pmic_probe(struct device *dev)
{
	struct axp803_pmic *this = to_axp803_pmic(dev);
	int err;

	if ((err = axp803_probe(&this->bus)))
		return err;

	/* Set up IRQs; disable all but the relevant ones. */
	for (uint8_t reg = IRQ_ENABLE_REG1; reg <= IRQ_ENABLE_REG6; ++reg) {
		uint8_t data = reg == IRQ_ENABLE_REG5 ? WANTED_IRQS : 0;
		if ((err = rsb_write(&this->bus, reg, data)))
			return err;
	}

	/* Now clear all IRQs. */
	for (uint8_t reg = IRQ_STATUS_REG1; reg <= IRQ_STATUS_REG6; ++reg) {
		if ((err = rsb_write(&this->bus, reg, GENMASK(7, 0))))
			return err;
	}

	/* Enable shutdown on PMIC overheat or >16 seconds button press;
	 * remember previous voltages when waking up from suspend. */
	if ((err = axp803_reg_setbits(&this->bus, PIN_FUNCTION_REG,
	                              GENMASK(3, 1))))
		return err;

	/* Register the NMI IRQ handler. */
	return irq_get(&container_of(dev, struct axp803_pmic, dev)->irq);
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
	.irq = {
		.dev     = &r_intc.dev,
		.irq     = IRQ_NMI,
		.handler = axp803_pmic_irq,
	},
};
