/*
 * Copyright © 2017-2020 The Crust Firmware Authors.
 * SPDX-License-Identifier: BSD-3-Clause OR GPL-2.0-only
 */

#include <debug.h>
#include <device.h>
#include <error.h>
#include <intrusive.h>
#include <mmio.h>
#include <regmap.h>
#include <util.h>
#include <clock/ccu.h>
#include <gpio/sunxi-gpio.h>
#include <regmap/sunxi-rsb.h>
#include <platform/devices.h>

#include "regmap.h"

#define RSB_CTRL_REG     0x00
#define RSB_CCR_REG      0x04
#define RSB_INT_EN_REG   0x08
#define RSB_STAT_REG     0x0c
#define RSB_ADDR_REG     0x10
#define RSB_DLEN_REG     0x18
#define RSB_DATA_REG     0x1c
#define RSB_LCR_REG      0x24
#define RSB_PMCR_REG     0x28
#define RSB_CMD_REG      0x2c
#define RSB_SADDR_REG    0x30

#define RSB_RTADDR(addr) ((addr) << 16)

#define I2C_BCAST_ADDR   0

#define PMIC_MODE_REG    0x3e
#define PMIC_MODE_VAL    0x7c

enum {
	RSB_SRTA = 0xe8,
	RSB_RD8  = 0x8b,
	RSB_RD16 = 0x9c,
	RSB_RD32 = 0xa6,
	RSB_WR8  = 0x4e,
	RSB_WR16 = 0x59,
	RSB_WR32 = 0x63,
};

static inline const struct sunxi_rsb *
to_sunxi_rsb(const struct device *dev)
{
	return container_of(dev, const struct sunxi_rsb, dev);
}

static uint16_t
sunxi_rsb_get_hwaddr(uint8_t rtaddr UNUSED)
{
	/* Currently only a primary PMIC is supported. */
	assert(rtaddr == 0x2d);

	return 0x3a3;
}

static int
sunxi_rsb_do_command(const struct sunxi_rsb *self, uint32_t addr, uint32_t cmd)
{
	mmio_write_32(self->regs + RSB_CMD_REG, cmd);
	mmio_write_32(self->regs + RSB_SADDR_REG, addr);
	mmio_write_32(self->regs + RSB_CTRL_REG, BIT(7));

	mmio_pollz_32(self->regs + RSB_CTRL_REG, BIT(7));

	return mmio_read_32(self->regs + RSB_STAT_REG) == BIT(0)
	       ? SUCCESS : EIO;
}

static int
sunxi_rsb_prepare(const struct regmap *map)
{
	const struct sunxi_rsb *self = to_sunxi_rsb(map->dev);
	uint32_t addr = RSB_RTADDR(map->id) | sunxi_rsb_get_hwaddr(map->id);

	/* Set the device's runtime address. */
	return sunxi_rsb_do_command(self, addr, RSB_SRTA);
}

static int
sunxi_rsb_read(const struct regmap *map, uint8_t addr, uint8_t *data)
{
	const struct sunxi_rsb *self = to_sunxi_rsb(map->dev);
	int err;

	mmio_write_32(self->regs + RSB_ADDR_REG, addr);

	if ((err = sunxi_rsb_do_command(self, RSB_RTADDR(map->id), RSB_RD8)))
		return err;

	*data = mmio_read_32(self->regs + RSB_DATA_REG);

	return SUCCESS;
}

static int
sunxi_rsb_write(const struct regmap *map, uint8_t addr, uint8_t data)
{
	const struct sunxi_rsb *self = to_sunxi_rsb(map->dev);

	mmio_write_32(self->regs + RSB_ADDR_REG, addr);
	mmio_write_32(self->regs + RSB_DATA_REG, data);

	return sunxi_rsb_do_command(self, RSB_RTADDR(map->id), RSB_WR8);
}

static void
sunxi_rsb_set_rate(const struct sunxi_rsb *self, uint32_t rate)
{
	uint32_t divider = (clock_get_rate(&self->clock) + rate) / (2 * rate);

	if (divider > 0)
		divider = divider - 1;
	if (divider > 0xff)
		divider = 0xff;

	mmio_write_32(self->regs + RSB_CCR_REG, 1U << 8 | divider);
}

static int
sunxi_rsb_probe(const struct device *dev)
{
	const struct sunxi_rsb *self = to_sunxi_rsb(dev);
	int err;

	if ((err = clock_get(&self->clock)))
		return err;
	if ((err = gpio_get(&self->pins[0])))
		goto err_put_clock;
	if ((err = gpio_get(&self->pins[1])))
		goto err_put_gpio0;

	/* Soft-reset the controller. */
	mmio_write_32(self->regs + RSB_CTRL_REG, BIT(0));
	mmio_pollz_32(self->regs + RSB_CTRL_REG, BIT(0));

	/* Set the bus clock rate to its default value (3 MHz). */
	sunxi_rsb_set_rate(self, 3000000);

	/* Switch all devices to RSB mode. */
	mmio_write_32(self->regs + RSB_PMCR_REG, I2C_BCAST_ADDR |
	              PMIC_MODE_REG << 8 | PMIC_MODE_VAL << 16 | BIT(31));
	mmio_pollz_32(self->regs + RSB_PMCR_REG, BIT(31));

	return SUCCESS;

err_put_gpio0:
	gpio_put(&self->pins[0]);
err_put_clock:
	clock_put(&self->clock);

	return err;
}

static void
sunxi_rsb_release(const struct device *dev)
{
	const struct sunxi_rsb *self = to_sunxi_rsb(dev);

	gpio_put(&self->pins[1]);
	gpio_put(&self->pins[0]);
	clock_put(&self->clock);
}

static const struct regmap_driver sunxi_rsb_driver = {
	.drv = {
		.probe   = sunxi_rsb_probe,
		.release = sunxi_rsb_release,
	},
	.ops = {
		.prepare = sunxi_rsb_prepare,
		.read    = sunxi_rsb_read,
		.write   = sunxi_rsb_write,
	},
};

const struct sunxi_rsb r_rsb = {
	.dev = {
		.name  = "r_rsb",
		.drv   = &sunxi_rsb_driver.drv,
		.state = DEVICE_STATE_INIT,
	},
	.clock = { .dev = &r_ccu.dev, .id = CLK_BUS_R_RSB },
	.pins  = {
		{
			.dev   = &r_pio.dev,
			.id    = SUNXI_GPIO_PIN(0, 0),
			.drive = DRIVE_10mA,
			.mode  = 2,
			.pull  = PULL_UP,
		},
		{
			.dev   = &r_pio.dev,
			.id    = SUNXI_GPIO_PIN(0, 1),
			.drive = DRIVE_10mA,
			.mode  = 2,
			.pull  = PULL_UP,
		},
	},
	.regs = DEV_R_RSB,
};
