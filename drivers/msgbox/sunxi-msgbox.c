/*
 * Copyright © 2017-2019 The Crust Firmware Authors.
 * SPDX-License-Identifier: BSD-3-Clause OR GPL-2.0-only
 */

#include <debug.h>
#include <dm.h>
#include <error.h>
#include <mmio.h>
#include <msgbox.h>
#include <scpi.h>
#include <stdbool.h>
#include <stddef.h>
#include <util.h>
#include <clock/sunxi-ccu.h>
#include <msgbox/sunxi-msgbox.h>
#include <platform/devices.h>

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
	assert(chan < SUNXI_MSGBOX_CHANS);

	/* Disable the receive IRQ. */
	mmio_clr_32(dev->regs + IRQ_EN_REG, RX_IRQ(chan));

	return SUCCESS;
}

static int
sunxi_msgbox_enable(struct device *dev, uint8_t chan)
{
	assert(chan < SUNXI_MSGBOX_CHANS);

	/* Clear and enable the receive IRQ. */
	mmio_write_32(dev->regs + IRQ_STAT_REG, RX_IRQ(chan));
	mmio_set_32(dev->regs + IRQ_EN_REG, RX_IRQ(chan));

	return SUCCESS;
}

static bool
sunxi_msgbox_last_tx_done(struct device *dev, uint8_t chan)
{
	assert(chan < SUNXI_MSGBOX_CHANS);

	return !(mmio_read_32(dev->regs + REMOTE_IRQ_STAT_REG) & RX_IRQ(chan));
}

static int
sunxi_msgbox_send(struct device *dev, uint8_t chan, uint32_t msg)
{
	assert(chan < SUNXI_MSGBOX_CHANS);

	/* Reject the message if the FIFO is full. */
	if (mmio_read_32(dev->regs + FIFO_STAT_REG(chan)) & FIFO_STAT_MASK)
		return EBUSY;
	mmio_write_32(dev->regs + MSG_DATA_REG(chan), msg);

	return SUCCESS;
}

static void
sunxi_msgbox_handle_msg(struct device *dev, uint8_t chan)
{
	uint32_t msg = mmio_read_32(dev->regs + MSG_DATA_REG(chan));

	switch (chan) {
	case MSGBOX_CHAN_SCPI_EL3_RX:
		scpi_receive_message(SCPI_CLIENT_EL3, msg);
		return;
	case MSGBOX_CHAN_SCPI_EL2_RX:
		scpi_receive_message(SCPI_CLIENT_EL2, msg);
		return;
	}

	debug("%s: %u: Unsolicited message 0x%08x", dev->name, chan, msg);
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
	int err;

	if ((err = dm_setup_clocks(dev, 1)))
		return err;

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
		.class = DM_CLASS_MSGBOX,
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
	.name   = "msgbox",
	.regs   = DEV_MSGBOX,
	.drv    = &sunxi_msgbox_driver.drv,
	.clocks = CLOCK_PARENT(ccu, CCU_CLOCK_MSGBOX),
};
