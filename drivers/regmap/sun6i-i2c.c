/*
 * Copyright © 2017-2020 The Crust Firmware Authors.
 * SPDX-License-Identifier: BSD-3-Clause OR GPL-2.0-only
 */

#include <delay.h>
#include <error.h>
#include <mmio.h>
#include <regmap.h>
#include <stdint.h>
#include <util.h>
#include <clock/ccu.h>
#include <gpio/sunxi-gpio.h>
#include <regmap/sun6i-i2c.h>
#include <platform/devices.h>

#include "regmap-i2c.h"

#define I2C_ADDR_REG  0x00
#define I2C_XADDR_REG 0x04
#define I2C_DATA_REG  0x08
#define I2C_CTRL_REG  0x0c
#define I2C_STAT_REG  0x10
#define I2C_CCR_REG   0x14
#define I2C_SRST_REG  0x18
#define I2C_EFR_REG   0x1c
#define I2C_LCR_REG   0x20

enum {
	START_COND_TX        = 0x08,
	START_COND_TX_REPEAT = 0x10,
	ADDR_WRITE_TX_ACK    = 0x18,
	ADDR_WRITE_TX_NACK   = 0x20,
	DATA_TX_ACK          = 0x28,
	DATA_TX_NACK         = 0x30,
	ADDR_READ_TX_ACK     = 0x40,
	ADDR_READ_TX_NACK    = 0x48,
	DATA_RX_ACK          = 0x50,
	DATA_RX_NACK         = 0x58,
	IDLE                 = 0xf8,
};

static int
sun6i_i2c_wait_idle(const struct simple_device *self)
{
	/* With a single master on the bus, this should only take one cycle. */
	int timeout = 2;

	while (mmio_read_32(self->regs + I2C_CTRL_REG) & (BIT(5) | BIT(4))) {
		/* 10μs is one 100kHz bus cycle. */
		udelay(10);
		if (timeout-- == 0)
			return EIO;
	}

	if (mmio_read_32(self->regs + I2C_STAT_REG) != IDLE)
		return EIO;

	return SUCCESS;
}

static int
sun6i_i2c_wait_start(const struct simple_device *self)
{
	/* With a single master on the bus, this should only take one cycle. */
	int timeout = 2;

	while (mmio_read_32(self->regs + I2C_CTRL_REG) & BIT(5)) {
		/* 10μs is one 100kHz bus cycle. */
		udelay(10);
		if (timeout-- == 0)
			return EIO;
	}

	return SUCCESS;
}

static int
sun6i_i2c_wait_state(const struct simple_device *self, uint8_t state)
{
	/* Wait for up to 8 transfer cycles, one ACK, and one extra cycle. */
	int timeout = 10;

	while (!(mmio_read_32(self->regs + I2C_CTRL_REG) & BIT(3))) {
		/* 10μs is one 100kHz bus cycle. */
		udelay(10);
		if (timeout-- == 0)
			return EIO;
	}

	if (mmio_read_32(self->regs + I2C_STAT_REG) != state)
		return EIO;

	return SUCCESS;
}

static int
sun6i_i2c_read(const struct regmap *map, uint8_t *data)
{
	const struct simple_device *self = to_simple_device(map->dev);
	int err;

	/* Disable sending an ACK and trigger a state change. */
	mmio_clrset_32(self->regs + I2C_CTRL_REG, BIT(2), BIT(3));

	/* Wait for data to arrive. */
	if ((err = sun6i_i2c_wait_state(self, DATA_RX_NACK)))
		return err;

	/* Read the data. */
	*data = mmio_read_32(self->regs + I2C_DATA_REG);

	return SUCCESS;
}

static int
sun6i_i2c_start(const struct regmap *map, uint8_t direction)
{
	const struct simple_device *self = to_simple_device(map->dev);
	uint8_t init_state = mmio_read_32(self->regs + I2C_STAT_REG);
	uint8_t state;
	int err;

	/* Send a start condition. */
	mmio_set_32(self->regs + I2C_CTRL_REG, BIT(5) | BIT(3));

	/* Wait for the start condition to be sent. */
	if ((err = sun6i_i2c_wait_start(self)))
		return err;

	/* Wait for the start state if the bus was previously idle; otherwise,
	 * wait for the repeated start state. */
	state = init_state == IDLE ? START_COND_TX : START_COND_TX_REPEAT;
	if ((err = sun6i_i2c_wait_state(self, state)))
		return err;

	/* Write the address and direction, then trigger a state change. */
	mmio_write_32(self->regs + I2C_DATA_REG, (map->id << 1) | direction);
	mmio_set_32(self->regs + I2C_CTRL_REG, BIT(3));

	/* Check for address acknowledgement. */
	state = direction == I2C_WRITE ? ADDR_WRITE_TX_ACK : ADDR_READ_TX_ACK;
	if ((err = sun6i_i2c_wait_state(self, state)))
		return err;

	return SUCCESS;
}

static void
sun6i_i2c_stop(const struct regmap *map)
{
	const struct simple_device *self = to_simple_device(map->dev);

	/* Send a stop condition. */
	mmio_set_32(self->regs + I2C_CTRL_REG, BIT(4) | BIT(3));

	/* Wait for the bus to go idle. */
	sun6i_i2c_wait_idle(self);
}

static int
sun6i_i2c_write(const struct regmap *map, uint8_t data)
{
	const struct simple_device *self = to_simple_device(map->dev);
	int err;

	/* Write data, then trigger a state change. */
	mmio_write_32(self->regs + I2C_DATA_REG, data);
	mmio_set_32(self->regs + I2C_CTRL_REG, BIT(3));

	/* Wait for data to be sent. */
	if ((err = sun6i_i2c_wait_state(self, DATA_TX_ACK)))
		return err;

	return SUCCESS;
}

static int
sun6i_i2c_probe(const struct device *dev)
{
	const struct simple_device *self = to_simple_device(dev);
	int err;

	if ((err = simple_device_probe(dev)))
		return err;

	/* Set I2C bus clock divider for 400 KHz operation. */
	mmio_write_32(self->regs + I2C_CCR_REG, 0x00000011);

	/* Clear slave address (this driver only supports master mode). */
	mmio_write_32(self->regs + I2C_ADDR_REG, 0);
	mmio_write_32(self->regs + I2C_XADDR_REG, 0);

	/* Enable I2C bus and stop any current transaction. Disable interrupts
	 * and don't send an ACK for received bytes. */
	mmio_write_32(self->regs + I2C_CTRL_REG, BIT(6) | BIT(4));

	/* Soft reset the controller. */
	mmio_set_32(self->regs + I2C_SRST_REG, BIT(0));

	/* Wait for the bus to go idle. */
	if ((err = sun6i_i2c_wait_idle(self)))
		goto err_release;

	return SUCCESS;

err_release:
	simple_device_release(dev);

	return err;
}

static const struct regmap_i2c_driver sun6i_i2c_driver = {
	.drv = {
		.drv = {
			.probe   = sun6i_i2c_probe,
			.release = simple_device_release,
		},
		.ops = {
			.prepare = regmap_i2c_prepare,
			.read    = regmap_i2c_read,
			.write   = regmap_i2c_write,
		},
	},
	.ops = {
		.read  = sun6i_i2c_read,
		.start = sun6i_i2c_start,
		.stop  = sun6i_i2c_stop,
		.write = sun6i_i2c_write,
	},
};

const struct simple_device r_i2c = {
	.dev = {
		.name  = "r_i2c",
		.drv   = &sun6i_i2c_driver.drv.drv,
		.state = DEVICE_STATE_INIT,
	},
	.clock = { .dev = &r_ccu.dev, .id = CLK_BUS_R_I2C },
#if CONFIG(I2C_PINS_PL0_PL1)
	.pins = SIMPLE_DEVICE_PINS_INIT {
		{
			.dev   = &r_pio.dev,
			.id    = SUNXI_GPIO_PIN(0, 0),
			.drive = DRIVE_10mA,
			.mode  = CONFIG_I2C_PIN_MODE_PL0_PL1,
			.pull  = PULL_UP,
		},
		{
			.dev   = &r_pio.dev,
			.id    = SUNXI_GPIO_PIN(0, 1),
			.drive = DRIVE_10mA,
			.mode  = CONFIG_I2C_PIN_MODE_PL0_PL1,
			.pull  = PULL_UP,
		},
	},
#elif CONFIG(I2C_PINS_PL8_PL9)
	.pins = SIMPLE_DEVICE_PINS_INIT {
		{
			.dev   = &r_pio.dev,
			.id    = SUNXI_GPIO_PIN(0, 8),
			.drive = DRIVE_10mA,
			.mode  = 2,
			.pull  = PULL_UP,
		},
		{
			.dev   = &r_pio.dev,
			.id    = SUNXI_GPIO_PIN(0, 9),
			.drive = DRIVE_10mA,
			.mode  = 2,
			.pull  = PULL_UP,
		},
	},
#endif
	.regs = DEV_R_I2C,
};
