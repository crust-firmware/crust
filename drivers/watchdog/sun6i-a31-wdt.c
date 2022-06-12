/*
 * Copyright © 2022 The Crust Firmware Authors.
 * SPDX-License-Identifier: BSD-3-Clause OR GPL-2.0-only
 */

#include <delay.h>
#include <error.h>
#include <intrusive.h>
#include <mmio.h>
#include <util.h>
#include <watchdog/sun6i-a31-wdt.h>
#include <platform/devices.h>

#include "watchdog.h"

#define WDOG_CTRL_REG      0x10
#define WDOG_CFG_REG       0x14
#define WDOG_MODE_REG      0x18

#define WDOG_RESTART_KEY   (0xa57 << 1)

#define WDOG_LONG_TIMEOUT  (5 << 4) /* 5.0 seconds */
#define WDOG_SHORT_TIMEOUT (0 << 4) /* 0.5 seconds */

static inline const struct sun6i_a31_wdt *
to_sun6i_a31_wdt(const struct device *dev)
{
	return container_of(dev, const struct sun6i_a31_wdt, dev);
}

static void
sun6i_a31_wdt_reset_system(const struct device *dev)
{
	const struct sun6i_a31_wdt *self = to_sun6i_a31_wdt(dev);

	mmio_write_32(self->regs + WDOG_MODE_REG, WDOG_SHORT_TIMEOUT | BIT(0));

	udelay(1000000);
}

static void
sun6i_a31_wdt_restart(const struct device *dev)
{
	const struct sun6i_a31_wdt *self = to_sun6i_a31_wdt(dev);

	mmio_write_32(self->regs + WDOG_CTRL_REG, WDOG_RESTART_KEY | BIT(0));
}

static int
sun6i_a31_wdt_probe(const struct device *dev)
{
	const struct sun6i_a31_wdt *self = to_sun6i_a31_wdt(dev);

	/* Enable system reset on timeout. */
	mmio_write_32(self->regs + WDOG_CFG_REG, 1);

	/* Start the watchdog with the default (long) timeout. */
	mmio_write_32(self->regs + WDOG_MODE_REG, WDOG_LONG_TIMEOUT | BIT(0));

	return SUCCESS;
}

static void
sun6i_a31_wdt_release(const struct device *dev)
{
	const struct sun6i_a31_wdt *self = to_sun6i_a31_wdt(dev);

	/* Stop the watchdog. */
	mmio_write_32(self->regs + WDOG_MODE_REG, 0);
}

static const struct watchdog_driver sun6i_a31_wdt_driver = {
	.drv = {
		.probe   = sun6i_a31_wdt_probe,
		.release = sun6i_a31_wdt_release,
	},
	.ops = {
		.reset_system = sun6i_a31_wdt_reset_system,
		.restart      = sun6i_a31_wdt_restart,
	},
};

const struct sun6i_a31_wdt r_wdog = {
	.dev = {
		.name  = "r_wdog",
		.drv   = &sun6i_a31_wdt_driver.drv,
		.state = DEVICE_STATE_INIT,
	},
	.regs = DEV_R_WDOG,
};
