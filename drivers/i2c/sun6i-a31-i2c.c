/*
 * Copyright © 2017-2018 The Crust Firmware Authors.
 * SPDX-License-Identifier: BSD-3-Clause OR GPL-2.0-only
 */

#include <delay.h>
#include <dm.h>
#include <error.h>
#include <i2c.h>
#include <mmio.h>
#include <stdbool.h>
#include <stdint.h>
#include <util.h>
#include <i2c/sun6i-a31-i2c.h>

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

static bool
sun6i_a31_i2c_wait_idle(struct device *dev)
{
	/* With a single master on the bus, this should only take one cycle. */
	int timeout = 2;

	while (mmio_read32(dev->regs + I2C_CTRL_REG) & (BIT(5) | BIT(4))) {
		/* 10μs is one 100kHz bus cycle. */
		udelay(10);
		if (timeout-- == 0)
			return false;
	}

	return mmio_read32(dev->regs + I2C_STAT_REG) == IDLE;
}

static bool
sun6i_a31_i2c_wait_start(struct device *dev)
{
	/* With a single master on the bus, this should only take one cycle. */
	int timeout = 2;

	while (mmio_read32(dev->regs + I2C_CTRL_REG) & BIT(5)) {
		/* 10μs is one 100kHz bus cycle. */
		udelay(10);
		if (timeout-- == 0)
			return false;
	}

	return true;
}

static bool
sun6i_a31_i2c_wait_state(struct device *dev, uint8_t state)
{
	/* Wait for up to 8 transfer cycles, one ACK, and one extra cycle. */
	int timeout = 10;

	while (!(mmio_read32(dev->regs + I2C_CTRL_REG) & BIT(3))) {
		/* 10μs is one 100kHz bus cycle. */
		udelay(10);
		if (timeout-- == 0)
			return false;
	}

	return mmio_read32(dev->regs + I2C_STAT_REG) == state;
}

static int
sun6i_a31_i2c_read(struct device *dev, uint8_t *data)
{
	/* Disable sending an ACK and trigger a state change. */
	mmio_clearsetbits32(dev->regs + I2C_CTRL_REG, BIT(2), BIT(3));

	/* Wait for data to arrive. */
	if (!sun6i_a31_i2c_wait_state(dev, DATA_RX_NACK))
		return EIO;

	/* Read the data. */
	*data = mmio_read32(dev->regs + I2C_DATA_REG);

	return SUCCESS;
}

static int
sun6i_a31_i2c_start(struct device *dev, uint8_t addr, uint8_t direction)
{
	uint8_t init_state = mmio_read32(dev->regs + I2C_STAT_REG);
	uint8_t state;

	/* Send a start condition. */
	mmio_setbits32(dev->regs + I2C_CTRL_REG, BIT(5) | BIT(3));

	/* Wait for the start condition to be sent. */
	if (!sun6i_a31_i2c_wait_start(dev))
		return EIO;

	/* Wait for the start state if the bus was previously idle; otherwise,
	 * wait for the repeated start state. */
	state = init_state == IDLE ? START_COND_TX : START_COND_TX_REPEAT;
	if (!sun6i_a31_i2c_wait_state(dev, state))
		return EIO;

	/* Write the address and direction, then trigger a state change. */
	mmio_write32(dev->regs + I2C_DATA_REG, (addr << 1) | direction);
	mmio_setbits32(dev->regs + I2C_CTRL_REG, BIT(3));

	/* Check for address acknowledgement. */
	state = direction == I2C_WRITE ? ADDR_WRITE_TX_ACK : ADDR_READ_TX_ACK;
	if (!sun6i_a31_i2c_wait_state(dev, state))
		return ENODEV;

	return SUCCESS;
}

static void
sun6i_a31_i2c_stop(struct device *dev)
{
	/* Send a stop condition. */
	mmio_setbits32(dev->regs + I2C_CTRL_REG, BIT(4) | BIT(3));

	/* Wait for the bus to go idle. */
	sun6i_a31_i2c_wait_idle(dev);
}

static int
sun6i_a31_i2c_write(struct device *dev, uint8_t data)
{
	/* Write data, then trigger a state change. */
	mmio_write32(dev->regs + I2C_DATA_REG, data);
	mmio_setbits32(dev->regs + I2C_CTRL_REG, BIT(3));

	/* Wait for data to be sent. */
	if (!sun6i_a31_i2c_wait_state(dev, DATA_TX_ACK))
		return EIO;

	return SUCCESS;
}

static int
sun6i_a31_i2c_probe(struct device *dev)
{
	int err;

	if ((err = dm_setup_clocks(dev, 1)))
		return err;

	/* Set port L pins 0-1 to I²C. */
	if ((err = dm_setup_pins(dev, I2C_NUM_PINS)))
		return err;

	/* Set I2C bus clock divider for 400 KHz operation. */
	mmio_write32(dev->regs + I2C_CCR_REG, 0x00000011);

	/* Clear slave address (this driver only supports master mode). */
	mmio_write32(dev->regs + I2C_ADDR_REG, 0);
	mmio_write32(dev->regs + I2C_XADDR_REG, 0);

	/* Enable I2C bus and stop any current transaction. Disable interrupts
	 * and don't send an ACK for received bytes. */
	mmio_write32(dev->regs + I2C_CTRL_REG, BIT(6) | BIT(4));

	/* Soft reset the controller. */
	mmio_setbits32(dev->regs + I2C_SRST_REG, BIT(0));

	/* Wait for the bus to go idle. */
	if (!sun6i_a31_i2c_wait_idle(dev))
		return EIO;

	return SUCCESS;
}

const struct i2c_driver sun6i_a31_i2c_driver = {
	.drv = {
		.class = DM_CLASS_I2C,
		.probe = sun6i_a31_i2c_probe,
	},
	.ops = {
		.read  = sun6i_a31_i2c_read,
		.start = sun6i_a31_i2c_start,
		.stop  = sun6i_a31_i2c_stop,
		.write = sun6i_a31_i2c_write,
	},
};
