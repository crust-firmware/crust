/*
 * Copyright © 2017-2019 The Crust Firmware Authors.
 * SPDX-License-Identifier: BSD-3-Clause OR GPL-2.0-only
 */

#include <compiler.h>
#include <delay.h>
#include <dm.h>
#include <error.h>
#include <mmio.h>
#include <msgbox.h>
#include <stdbool.h>
#include <stddef.h>
#include <util.h>
#include <wallclock.h>
#include <msgbox/sunxi-msgbox.h>
#include <platform/devices.h>
#include <platform/time.h>

#define CTRL_REG0           0x0000
#define CTRL_REG1           0x0004
#define CTRL_NORMAL         0x01100110

#define IRQ_EN_REG          0x0040
#define IRQ_STAT_REG        0x0050
#define REMOTE_IRQ_EN_REG   0x0060
#define REMOTE_IRQ_STAT_REG 0x0070
#define RX_IRQ(n)           BIT(0 + 2 * (n))
#define RX_IRQ_MASK         0x5555
#define TX_IRQ(n)           BIT(1 + 2 * (n))
#define TX_IRQ_MASK         0xaaaa

#define FIFO_STAT_REG(n)    (0x0100 + 0x4 * (n))
#define FIFO_STAT_MASK      BIT(0)

#define MSG_STAT_REG(n)     (0x0140 + 0x4 * (n))
#define MSG_STAT_MASK       GENMASK(2, 0)

#define MSG_DATA_REG(n)     (0x0180 + 0x4 * (n))

static uint32_t address;

noreturn void start(void);

static bool
sunxi_msgbox_peek_data(struct device *dev, uint8_t chan)
{
	return mmio_read_32(dev->regs + MSG_STAT_REG(chan)) & MSG_STAT_MASK;
}

static void
sunxi_msgbox_ack_rx(struct device *dev, uint8_t chan)
{
	mmio_write_32(dev->regs + IRQ_STAT_REG, RX_IRQ(chan));
}

static int
sunxi_msgbox_disable(struct device *dev, uint8_t chan)
{
	/* Disable the receive IRQ. */
	mmio_clr_32(dev->regs + IRQ_EN_REG, RX_IRQ(chan));

	return SUCCESS;
}

static int
sunxi_msgbox_enable(struct device *dev, uint8_t chan)
{
	/* Clear and enable the receive IRQ. */
	mmio_write_32(dev->regs + IRQ_STAT_REG, RX_IRQ(chan));
	mmio_set_32(dev->regs + IRQ_EN_REG, RX_IRQ(chan));

	return SUCCESS;
}

static bool
sunxi_msgbox_last_tx_done(struct device *dev, uint8_t chan)
{
	return !(mmio_read_32(dev->regs + REMOTE_IRQ_STAT_REG) & RX_IRQ(chan));
}

static int
sunxi_msgbox_send(struct device *dev, uint8_t chan, uint32_t msg)
{
	/* Reject the message if the FIFO is full. */
	if (mmio_read_32(dev->regs + FIFO_STAT_REG(chan)) & FIFO_STAT_MASK)
		return EBUSY;
	mmio_write_32(dev->regs + MSG_DATA_REG(chan), msg);

	return SUCCESS;
}

static void
sunxi_msgbox_handle_msg(struct device *dev, uint8_t chan)
{
	uint32_t msg    = mmio_read_32(dev->regs + MSG_DATA_REG(chan));
	uint16_t opcode = msg >> 16;
	uint16_t data   = msg & 0xffff;
	uint32_t time;

	/* Acknowledge the request. */
	sunxi_msgbox_ack_rx(dev, chan);

	/* Send replies on the other channel in the pair. */
	chan ^= 1;

	switch (opcode) {
	case 0:
		/* Magic value */
		sunxi_msgbox_send(dev, chan, 0x1a2a3a4a);
		break;
	case 1:
		/* Version */
		sunxi_msgbox_send(dev, chan, 0x00000101);
		break;
	case 2:
		/* Loopback */
		sunxi_msgbox_send(dev, chan, data);
		break;
	case 3:
		/* Loopback inverted */
		sunxi_msgbox_send(dev, chan, ~data);
		break;
	case 4:
		/* Current time (seconds) */
		time = wallclock_read() >> 9;
		sunxi_msgbox_send(dev, chan, time / (REFCLK_HZ >> 9));
		break;
	case 5:
		/* Current time (ticks) */
		sunxi_msgbox_send(dev, chan, wallclock_read());
		break;
	case 6:
		/* Delay (microseconds) */
		udelay(data);
		sunxi_msgbox_send(dev, chan, 0);
		break;
	case 7:
		/* Delay (milliseconds) */
		udelay(1000 * data);
		sunxi_msgbox_send(dev, chan, 0);
		break;
	case 8:
		/* Set address (low half) */
		address = (address & 0xffff0000U) | data;
		sunxi_msgbox_send(dev, chan, address);
		break;
	case 9:
		/* Set address (high half) */
		address = (address & 0xffff) | (data << 16);
		sunxi_msgbox_send(dev, chan, address);
		break;
	case 10:
		/* Read address */
		sunxi_msgbox_send(dev, chan, mmio_read_32(address));
		break;
	case 11:
		/* Write address (sign extended) */
		mmio_write_32(address, (int16_t)data);
		sunxi_msgbox_send(dev, chan, mmio_read_32(address));
		break;
	case 16:
		/* Reset the firmware */
		start();
		break;
	default:
		/* Error value */
		sunxi_msgbox_send(dev, chan, -1U);
		break;
	}
}

static void
sunxi_msgbox_poll(struct device *dev)
{
	uint32_t status = mmio_read_32(dev->regs + IRQ_STAT_REG);

	if (!(status & RX_IRQ_MASK))
		return;

	for (uint8_t chan = 0; chan < SUNXI_MSGBOX_CHANS; chan += 2) {
		if (status & RX_IRQ(chan)) {
			if (sunxi_msgbox_peek_data(dev, chan))
				sunxi_msgbox_handle_msg(dev, chan);
		}
	}
}

static int
sunxi_msgbox_probe(struct device *dev)
{
	/* Enable the msgbox clock and reset. */
	mmio_set_32(DEV_CCU + 0x0064, BIT(21));
	mmio_set_32(DEV_CCU + 0x02c4, BIT(21));

	/* Set even channels ARM -> SCP and odd channels SCP -> ARM. */
	mmio_write_32(dev->regs + CTRL_REG0, CTRL_NORMAL);
	mmio_write_32(dev->regs + CTRL_REG1, CTRL_NORMAL);

	/* Drain messages in RX channels (required to clear IRQs). */
	for (uint8_t chan = 0; chan < SUNXI_MSGBOX_CHANS; chan += 2) {
		while (sunxi_msgbox_peek_data(dev, chan))
			mmio_read_32(dev->regs + MSG_DATA_REG(chan));
	}

	/* Disable and clear all IRQ. */
	mmio_write_32(dev->regs + IRQ_EN_REG, 0);
	mmio_write_32(dev->regs + IRQ_STAT_REG, GENMASK(15, 0));

	return SUCCESS;
}

static const struct msgbox_driver sunxi_msgbox_driver = {
	.drv = {
		.poll  = sunxi_msgbox_poll,
		.probe = sunxi_msgbox_probe,
	},
	.ops = {
		.ack_rx       = sunxi_msgbox_ack_rx,
		.disable      = sunxi_msgbox_disable,
		.enable       = sunxi_msgbox_enable,
		.last_tx_done = sunxi_msgbox_last_tx_done,
		.send         = sunxi_msgbox_send,
	},
};

struct device msgbox = {
	.name = "msgbox",
	.regs = DEV_MSGBOX,
	.drv  = &sunxi_msgbox_driver.drv,
};
