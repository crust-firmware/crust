/*
 * Copyright © 2017-2020 The Crust Firmware Authors.
 * SPDX-License-Identifier: BSD-3-Clause OR GPL-2.0-only
 */

#include <delay.h>
#include <device.h>
#include <error.h>
#include <intrusive.h>
#include <mmio.h>
#include <regmap.h>
#include <stdbool.h>
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

static inline const struct sun6i_i2c *
to_sun6i_i2c(const struct device *dev)
{
	return container_of(dev, const struct sun6i_i2c, dev);
}

static bool
sun6i_i2c_wait_idle(const struct sun6i_i2c *self)
{
	/* With a single master on the bus, this should only take one cycle. */
	int timeout = 2;

	while (mmio_read_32(self->regs + I2C_CTRL_REG) & (BIT(5) | BIT(4))) {
		/* 10μs is one 100kHz bus cycle. */
		udelay(10);
		if (timeout-- == 0)
			return false;
	}

	return mmio_read_32(self->regs + I2C_STAT_REG) == IDLE;
}

static bool
sun6i_i2c_wait_start(const struct sun6i_i2c *self)
{
	/* With a single master on the bus, this should only take one cycle. */
	int timeout = 2;

	while (mmio_read_32(self->regs + I2C_CTRL_REG) & BIT(5)) {
		/* 10μs is one 100kHz bus cycle. */
		udelay(10);
		if (timeout-- == 0)
			return false;
	}

	return true;
}

static bool
sun6i_i2c_wait_state(const struct sun6i_i2c *self, uint8_t state)
{
	/* Wait for up to 8 transfer cycles, one ACK, and one extra cycle. */
	int timeout = 10;

	while (!(mmio_read_32(self->regs + I2C_CTRL_REG) & BIT(3))) {
		/* 10μs is one 100kHz bus cycle. */
		udelay(10);
		if (timeout-- == 0)
			return false;
	}

	return mmio_read_32(self->regs + I2C_STAT_REG) == state;
}

static int
sun6i_i2c_read(const struct regmap *map, uint8_t *data)
{
	const struct sun6i_i2c *self = to_sun6i_i2c(map->dev);

	/* Disable sending an ACK and trigger a state change. */
	mmio_clrset_32(self->regs + I2C_CTRL_REG, BIT(2), BIT(3));

	/* Wait for data to arrive. */
	if (!sun6i_i2c_wait_state(self, DATA_RX_NACK))
		return EIO;

	/* Read the data. */
	*data = mmio_read_32(self->regs + I2C_DATA_REG);

	return SUCCESS;
}

static int
sun6i_i2c_start(const struct regmap *map, uint8_t direction)
{
	const struct sun6i_i2c *self = to_sun6i_i2c(map->dev);
	uint8_t init_state = mmio_read_32(self->regs + I2C_STAT_REG);
	uint8_t state;

	/* Send a start condition. */
	mmio_set_32(self->regs + I2C_CTRL_REG, BIT(5) | BIT(3));

	/* Wait for the start condition to be sent. */
	if (!sun6i_i2c_wait_start(self))
		return EIO;

	/* Wait for the start state if the bus was previously idle; otherwise,
	 * wait for the repeated start state. */
	state = init_state == IDLE ? START_COND_TX : START_COND_TX_REPEAT;
	if (!sun6i_i2c_wait_state(self, state))
		return EIO;

	/* Write the address and direction, then trigger a state change. */
	mmio_write_32(self->regs + I2C_DATA_REG, (map->id << 1) | direction);
	mmio_set_32(self->regs + I2C_CTRL_REG, BIT(3));

	/* Check for address acknowledgement. */
	state = direction == I2C_WRITE ? ADDR_WRITE_TX_ACK : ADDR_READ_TX_ACK;
	if (!sun6i_i2c_wait_state(self, state))
		return ENODEV;

	return SUCCESS;
}

static void
sun6i_i2c_stop(const struct regmap *map)
{
	const struct sun6i_i2c *self = to_sun6i_i2c(map->dev);

	/* Send a stop condition. */
	mmio_set_32(self->regs + I2C_CTRL_REG, BIT(4) | BIT(3));

	/* Wait for the bus to go idle. */
	sun6i_i2c_wait_idle(self);
}

static int
sun6i_i2c_write(const struct regmap *map, uint8_t data)
{
	const struct sun6i_i2c *self = to_sun6i_i2c(map->dev);

	/* Write data, then trigger a state change. */
	mmio_write_32(self->regs + I2C_DATA_REG, data);
	mmio_set_32(self->regs + I2C_CTRL_REG, BIT(3));

	/* Wait for data to be sent. */
	if (!sun6i_i2c_wait_state(self, DATA_TX_ACK))
		return EIO;

	return SUCCESS;
}

static int
sun6i_i2c_probe(const struct device *dev)
{
	const struct sun6i_i2c *self = to_sun6i_i2c(dev);
	int err;

	if ((err = clock_get(&self->clock)))
		return err;
	if ((err = gpio_get(&self->pins[0])))
		goto err_put_clock;
	if ((err = gpio_get(&self->pins[1])))
		goto err_put_gpio0;

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
	if (!sun6i_i2c_wait_idle(self)) {
		err = EIO;
		goto err_put_gpio1;
	}

	return SUCCESS;

err_put_gpio1:
	gpio_put(&self->pins[1]);
err_put_gpio0:
	gpio_put(&self->pins[0]);
err_put_clock:
	clock_put(&self->clock);

	return err;
}

static void
sun6i_i2c_release(const struct device *dev)
{
	const struct sun6i_i2c *self = to_sun6i_i2c(dev);

	gpio_put(&self->pins[1]);
	gpio_put(&self->pins[0]);
	clock_put(&self->clock);
}

static const struct regmap_i2c_driver sun6i_i2c_driver = {
	.drv = {
		.drv = {
			.probe   = sun6i_i2c_probe,
			.release = sun6i_i2c_release,
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

const struct sun6i_i2c r_i2c = {
	.dev = {
		.name  = "r_i2c",
		.drv   = &sun6i_i2c_driver.drv.drv,
		.state = DEVICE_STATE_INIT,
	},
	.clock = { .dev = &r_ccu.dev, .id = CLK_BUS_R_I2C },
	.pins  = {
#if CONFIG(I2C_PINS_PL0_PL1)
		{
			.dev   = &r_pio.dev,
			.id    = SUNXI_GPIO_PIN(0, 0),
			.drive = DRIVE_10mA,
			.mode  = CONFIG(SOC_H5) ? 2 : 3,
			.pull  = PULL_UP,
		},
		{
			.dev   = &r_pio.dev,
			.id    = SUNXI_GPIO_PIN(0, 1),
			.drive = DRIVE_10mA,
			.mode  = CONFIG(SOC_H5) ? 2 : 3,
			.pull  = PULL_UP,
		},
#elif CONFIG(I2C_PINS_PL8_PL9)
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
#endif
	},
	.regs = DEV_R_I2C,
};
