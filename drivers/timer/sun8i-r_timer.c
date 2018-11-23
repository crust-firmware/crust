/*
 * Copyright © 2017-2018 The Crust Firmware Authors.
 * SPDX-License-Identifier: BSD-3-Clause OR GPL-2.0-only
 */

#include <delay.h>
#include <dm.h>
#include <error.h>
#include <mmio.h>
#include <stdbool.h>
#include <stddef.h>
#include <timer.h>
#include <timer/sun8i-r_timer.h>

#define IRQ_EN_REG      0x0000
#define IRQ_STATUS_REG  0x0004

#define CONTROL_REG(n)  (0x20 * (n) + 0x20)
#define INTERVAL_REG(n) (0x20 * (n) + 0x24)
#define VALUE_REG(n)    (0x20 * (n) + 0x28)

static int
sun8i_r_timer_get_timeout(struct device *dev, uint32_t *timeout)
{
	uintptr_t index = dev->drvdata;

	if (mmio_read32(dev->regs + CONTROL_REG(index)) & BIT(0))
		*timeout = mmio_read32(dev->regs + VALUE_REG(index));
	else
		*timeout = 0;

	return SUCCESS;
}

static int
sun8i_r_timer_set_timeout(struct device *dev, uint32_t timeout)
{
	bool running;
	uintptr_t index = dev->drvdata;

	running = mmio_read32(dev->regs + CONTROL_REG(index)) & BIT(0);
	/* Pause the timer if it is already running. */
	if (running) {
		mmio_clearbits32(dev->regs + CONTROL_REG(index), BIT(0));
		/* Must wait at least two cycles before resuming the timer. */
		udelay(1);
	}
	mmio_write32(dev->regs + INTERVAL_REG(index), timeout);
	/* If the timer is paused (not stopped), also set the reload bit. */
	mmio_setbits32(dev->regs + CONTROL_REG(index),
	               running ? BITMASK(0, 2) : BIT(0));

	return SUCCESS;
}

void
sun8i_r_timer_irq(void *param)
{
	struct device *dev = param;
	uintptr_t index    = dev->drvdata;

	/* Clear the pending IRQ. */
	mmio_write32(dev->regs + IRQ_STATUS_REG, BIT(index));

	timer_tick();
}

static int
sun8i_r_timer_probe(struct device *dev)
{
	int err;
	uintptr_t index = dev->drvdata;

	if ((err = dm_setup_clocks(dev, 1)))
		return err;

	/* Stop the timer and set it to 24MHz one-shot mode. */
	mmio_clearsetbits32(dev->regs + CONTROL_REG(index),
	                    BITMASK(0, 8), BIT(2) | BIT(7));

	/* Enable and clear the IRQ. */
	mmio_setbits32(dev->regs + IRQ_EN_REG, BIT(index));
	mmio_write32(dev->regs + IRQ_STATUS_REG, BIT(index));

	/* Register and enable the IRQ at the interrupt controller. */
	if ((err = dm_setup_irq(dev, sun8i_r_timer_irq)))
		return err;

	/* Register this device with the timer framework. */
	if ((err = timer_register_device(dev)))
		return err;

	return SUCCESS;
}

const struct timer_driver sun8i_r_timer_driver = {
	.drv = {
		.class = DM_CLASS_TIMER,
		.probe = sun8i_r_timer_probe,
	},
	.ops = {
		.get_timeout = sun8i_r_timer_get_timeout,
		.set_timeout = sun8i_r_timer_set_timeout,
	},
};
