/*
 * Copyright © 2017-2018 The Crust Firmware Authors.
 * SPDX-License-Identifier: (BSD-3-Clause OR GPL-2.0)
 */

#include <debug.h>
#include <dm.h>
#include <error.h>
#include <spr.h>
#include <drivers/irqchip.h>

static struct device *irqchip_device;

int
irqchip_device_register(struct device *dev)
{
	if (irqchip_device)
		return EEXIST;
	irqchip_device = dev;

	/* Enable the CPU external interrupt input. */
	mtspr(SPR_SYS_SR_ADDR, SPR_SYS_SR_IEE_SET(mfspr(SPR_SYS_SR_ADDR), 1));

	return SUCCESS;
}

int
irqchip_irq(void)
{
	if (!irqchip_device)
		panic("Interrupt with no irqchip registered");

	return IRQCHIP_OPS(irqchip_device)->irq(irqchip_device);
}
