/*
 * Copyright © 2017-2019 The Crust Firmware Authors.
 * SPDX-License-Identifier: BSD-3-Clause OR GPL-2.0-only
 */

#include <debug.h>
#include <device.h>
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

static inline const struct sunxi_msgbox *
to_sunxi_msgbox(const struct device *dev)
{
	return container_of(dev, struct sunxi_msgbox, dev);
}

static bool
sunxi_msgbox_peek_data(const struct device *dev, uint8_t chan)
{
	const struct sunxi_msgbox *self = to_sunxi_msgbox(dev);

	return mmio_read_32(self->regs + MSG_STAT_REG(chan)) & MSG_STAT_MASK;
}

static void
sunxi_msgbox_ack_rx(const struct device *dev, uint8_t chan)
{
	const struct sunxi_msgbox *self = to_sunxi_msgbox(dev);

	mmio_write_32(self->regs + IRQ_STAT_REG, RX_IRQ(chan));
}

static bool
sunxi_msgbox_last_tx_done(const struct device *dev, uint8_t chan)
{
	const struct sunxi_msgbox *self = to_sunxi_msgbox(dev);

	assert(chan < SUNXI_MSGBOX_CHANS);

	return !(mmio_read_32(self->regs + REMOTE_IRQ_STAT_REG) &
	         RX_IRQ(chan));
}

static int
sunxi_msgbox_receive(const struct device *dev, uint8_t chan, uint32_t *msg)
{
	const struct sunxi_msgbox *self = to_sunxi_msgbox(dev);

	assert(chan < SUNXI_MSGBOX_CHANS);

	/* Check if a new message is available before reading it. */
	if (!sunxi_msgbox_peek_data(dev, chan))
		return ENOENT;
	*msg = mmio_read_32(self->regs + MSG_DATA_REG(chan));

	return SUCCESS;
}

static int
sunxi_msgbox_send(const struct device *dev, uint8_t chan, uint32_t msg)
{
	const struct sunxi_msgbox *self = to_sunxi_msgbox(dev);

	assert(chan < SUNXI_MSGBOX_CHANS);

	/* Reject the message if the FIFO is full. */
	if (mmio_read_32(self->regs + FIFO_STAT_REG(chan)) & FIFO_STAT_MASK)
		return EBUSY;
	mmio_write_32(self->regs + MSG_DATA_REG(chan), msg);

	return SUCCESS;
}

static void
sunxi_msgbox_handle_msg(const struct device *dev, uint8_t chan)
{
	const struct sunxi_msgbox *self = to_sunxi_msgbox(dev);
	uint32_t msg = mmio_read_32(self->regs + MSG_DATA_REG(chan));

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
sunxi_msgbox_poll(const struct device *dev)
{
	const struct sunxi_msgbox *self = to_sunxi_msgbox(dev);
	uint32_t status = mmio_read_32(self->regs + IRQ_STAT_REG);

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
sunxi_msgbox_probe(const struct device *dev)
{
	const struct sunxi_msgbox *self = to_sunxi_msgbox(dev);
	int err;

	if ((err = clock_get(&self->clock)))
		return err;

	/* Set even channels ARM -> SCP and odd channels SCP -> ARM. */
	mmio_write_32(self->regs + CTRL_REG0, CTRL_NORMAL);
	mmio_write_32(self->regs + CTRL_REG1, CTRL_NORMAL);

	/* Drain messages in RX channels (required to clear IRQs). */
	for (uint8_t chan = 0; chan < SUNXI_MSGBOX_CHANS; chan += 2) {
		while (sunxi_msgbox_peek_data(dev, chan))
			mmio_read_32(self->regs + MSG_DATA_REG(chan));
	}

	/* Disable and clear all IRQ. */
	mmio_write_32(self->regs + IRQ_EN_REG, 0);
	mmio_write_32(self->regs + IRQ_STAT_REG, GENMASK(15, 0));

	return SUCCESS;
}

static const struct msgbox_driver sunxi_msgbox_driver = {
	.drv = {
		.poll  = sunxi_msgbox_poll,
		.probe = sunxi_msgbox_probe,
	},
	.ops = {
		.ack_rx       = sunxi_msgbox_ack_rx,
		.last_tx_done = sunxi_msgbox_last_tx_done,
		.receive      = sunxi_msgbox_receive,
		.send         = sunxi_msgbox_send,
	},
};

const struct sunxi_msgbox msgbox = {
	.dev = {
		.name  = "msgbox",
		.drv   = &sunxi_msgbox_driver.drv,
		.state = DEVICE_STATE_INIT,
	},
	.clock = { .dev = &ccu.dev, .id = CCU_CLOCK_MSGBOX },
	.regs  = DEV_MSGBOX,
};
