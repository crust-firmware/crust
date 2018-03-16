/*
 * Copyright © 2017-2018 The Crust Firmware Authors.
 * SPDX-License-Identifier: (BSD-3-Clause OR GPL-2.0)
 */

#include <debug.h>
#include <dm.h>
#include <error.h>
#include <irqchip.h>
#include <spr.h>
#include <stddef.h>

static struct device *irqchip;

void
irqchip_irq(void)
{
	assert(irqchip != NULL);

	IRQCHIP_OPS(irqchip)->irq(irqchip);
}

int
irqchip_register_device(struct device *dev)
{
	if (irqchip != NULL)
		return EEXIST;

	irqchip = dev;

	/* Enable the CPU external interrupt input. */
	mtspr(SPR_SYS_SR_ADDR, SPR_SYS_SR_IEE_SET(mfspr(SPR_SYS_SR_ADDR), 1));

	return SUCCESS;
}
