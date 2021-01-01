/*
 * Copyright © 2017-2021 The Crust Firmware Authors.
 * SPDX-License-Identifier: BSD-3-Clause OR GPL-2.0-only
 */

#include <error.h>
#include <mmio.h>
#include <util.h>
#include <clock/ccu.h>
#include <gpio/sunxi-gpio.h>
#include <regmap/sunxi-rsb.h>
#include <platform/devices.h>

#include "regmap.h"

#define RSB_CTRL_REG   0x00
#define RSB_CCR_REG    0x04
#define RSB_INT_EN_REG 0x08
#define RSB_STAT_REG   0x0c
#define RSB_ADDR_REG   0x10
#define RSB_DLEN_REG   0x18
#define RSB_DATA_REG   0x1c
#define RSB_LCR_REG    0x24
#define RSB_PMCR_REG   0x28
#define RSB_CMD_REG    0x2c
#define RSB_SADDR_REG  0x30

#define I2C_BCAST_ADDR 0

#define PMIC_MODE_REG  0x3e
#define PMIC_MODE_VAL  0x7c

enum {
	RSB_SRTA = 0xe8,
	RSB_RD8  = 0x8b,
	RSB_RD16 = 0x9c,
	RSB_RD32 = 0xa6,
	RSB_WR8  = 0x4e,
	RSB_WR16 = 0x59,
	RSB_WR32 = 0x63,
};

static int
sunxi_rsb_do_command(const struct regmap *map, uint32_t cmd)
{
	const struct simple_device *self = to_simple_device(map->dev);

	mmio_write_32(self->regs + RSB_CMD_REG, cmd);
	mmio_write_32(self->regs + RSB_SADDR_REG, map->id);
	mmio_write_32(self->regs + RSB_CTRL_REG, BIT(7));

	mmio_pollz_32(self->regs + RSB_CTRL_REG, BIT(7));

	return mmio_read_32(self->regs + RSB_STAT_REG) == BIT(0)
	       ? SUCCESS : EIO;
}

static int
sunxi_rsb_prepare(const struct regmap *map)
{
	/* Set the device's runtime address. */
	return sunxi_rsb_do_command(map, RSB_SRTA);
}

static int
sunxi_rsb_read(const struct regmap *map, uint8_t addr, uint8_t *data)
{
	const struct simple_device *self = to_simple_device(map->dev);
	int err;

	mmio_write_32(self->regs + RSB_ADDR_REG, addr);

	if ((err = sunxi_rsb_do_command(map, RSB_RD8)))
		return err;

	*data = mmio_read_32(self->regs + RSB_DATA_REG);

	return SUCCESS;
}

static int
sunxi_rsb_write(const struct regmap *map, uint8_t addr, uint8_t data)
{
	const struct simple_device *self = to_simple_device(map->dev);

	mmio_write_32(self->regs + RSB_ADDR_REG, addr);
	mmio_write_32(self->regs + RSB_DATA_REG, data);

	return sunxi_rsb_do_command(map, RSB_WR8);
}

static void
sunxi_rsb_set_rate(const struct simple_device *self, uint32_t rate)
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
	const struct simple_device *self = to_simple_device(dev);
	int err;

	if ((err = simple_device_probe(dev)))
		return err;

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
}

static const struct regmap_driver sunxi_rsb_driver = {
	.drv = {
		.probe   = sunxi_rsb_probe,
		.release = simple_device_release,
	},
	.ops = {
		.prepare = sunxi_rsb_prepare,
		.read    = sunxi_rsb_read,
		.write   = sunxi_rsb_write,
	},
};

const struct simple_device r_rsb = {
	.dev = {
		.name  = "r_rsb",
		.drv   = &sunxi_rsb_driver.drv,
		.state = DEVICE_STATE_INIT,
	},
	.clock = { .dev = &r_ccu.dev, .id = CLK_BUS_R_RSB },
	.pins  = SIMPLE_DEVICE_PINS_INIT {
		{
			.dev   = &r_pio.dev,
			.id    = SUNXI_GPIO_PIN(0, 0),
			.drive = DRIVE_30mA,
			.mode  = 2,
			.pull  = PULL_UP,
		},
		{
			.dev   = &r_pio.dev,
			.id    = SUNXI_GPIO_PIN(0, 1),
			.drive = DRIVE_30mA,
			.mode  = 2,
			.pull  = PULL_UP,
		},
	},
	.regs = DEV_R_RSB,
};
