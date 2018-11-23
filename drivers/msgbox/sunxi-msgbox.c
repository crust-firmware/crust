/*
 * Copyright © 2017-2018 The Crust Firmware Authors.
 * SPDX-License-Identifier: BSD-3-Clause OR GPL-2.0-only
 */

#include <debug.h>
#include <dm.h>
#include <error.h>
#include <mmio.h>
#include <msgbox.h>
#include <stdbool.h>
#include <stddef.h>
#include <util.h>
#include <msgbox/sunxi-msgbox.h>

/* These macros take a virtual channel number. */
#define CTRL_REG(n)           (0x0000 + 0x4 * ((n) / 2))
#define CTRL_MASK(n)          (0x1111 << 16 * ((n) % 2))
#define CTRL_SET(n)           (0x0110 << 16 * ((n) % 2))

#define IRQ_EN_REG            0x0040
#define IRQ_STATUS_REG        0x0050
#define RX_IRQ(n)             BIT(0 + 4 * (n))
#define TX_IRQ(n)             BIT(3 + 4 * (n))

#define REMOTE_IRQ_EN_REG     0x0060
#define REMOTE_IRQ_STATUS_REG 0x0070
#define REMOTE_RX_IRQ(n)      BIT(2 + 4 * (n))
#define REMOTE_TX_IRQ(n)      BIT(1 + 4 * (n))

#define RX_FIFO_STATUS_REG(n) (0x0100 + 0x8 * (n))
#define TX_FIFO_STATUS_REG(n) (0x0104 + 0x8 * (n))
#define FIFO_STATUS_MASK      BIT(0)

#define RX_MSG_STATUS_REG(n)  (0x0140 + 0x8 * (n))
#define TX_MSG_STATUS_REG(n)  (0x0144 + 0x8 * (n))
#define MSG_STATUS_MASK       BITMASK(0, 3)

#define RX_MSG_DATA_REG(n)    (0x0180 + 0x8 * (n))
#define TX_MSG_DATA_REG(n)    (0x0184 + 0x8 * (n))

static bool sunxi_msgbox_tx_pending(struct device *dev, uint8_t chan);

static inline msgbox_handler *
get_handler(struct device *dev, uint8_t chan)
{
	return ((msgbox_handler **)dev->drvdata)[chan];
}

static inline void
set_handler(struct device *dev, uint8_t chan, msgbox_handler *handler)
{
	((msgbox_handler **)dev->drvdata)[chan] = handler;
}

static bool
sunxi_msgbox_peek_data(struct device *dev, uint8_t chan)
{
	uint32_t reg;

	reg = mmio_read32(dev->regs + RX_MSG_STATUS_REG(chan));

	return (reg & MSG_STATUS_MASK) > 0;
}

static int
sunxi_msgbox_disable(struct device *dev, uint8_t chan)
{
	assert(chan < SUNXI_MSGBOX_CHANS);
	assert(get_handler(dev, chan) != NULL);

	/* Disable the receive interrupt. */
	mmio_clearbits32(dev->regs + IRQ_EN_REG, RX_IRQ(chan));

	set_handler(dev, chan, NULL);

	return SUCCESS;
}

static int
sunxi_msgbox_enable(struct device *dev, uint8_t chan,
                    msgbox_handler *handler)
{
	assert(chan < SUNXI_MSGBOX_CHANS);
	assert(handler != NULL);

	if (get_handler(dev, chan) != NULL)
		return EEXIST;
	set_handler(dev, chan, handler);

	/* Ensure FIFO directions are set properly. */
	mmio_clearsetbits32(dev->regs + CTRL_REG(chan),
	                    CTRL_MASK(chan), CTRL_SET(chan));

	/* Clear existing messages in the receive FIFO. */
	while (sunxi_msgbox_peek_data(dev, chan))
		mmio_read32(dev->regs + RX_MSG_DATA_REG(chan));

	/* Clear and enable the receive interrupt. */
	mmio_setbits32(dev->regs + IRQ_STATUS_REG, RX_IRQ(chan));
	mmio_setbits32(dev->regs + IRQ_EN_REG, RX_IRQ(chan));

	return SUCCESS;
}

static int
sunxi_msgbox_send(struct device *dev, uint8_t chan, uint32_t msg)
{
	assert(chan < SUNXI_MSGBOX_CHANS);

	if (sunxi_msgbox_tx_pending(dev, chan))
		return EBUSY;
	mmio_write32(dev->regs + TX_MSG_DATA_REG(chan), msg);

	return SUCCESS;
}

static bool
sunxi_msgbox_tx_pending(struct device *dev, uint8_t chan)
{
	uint32_t reg;

	assert(chan < SUNXI_MSGBOX_CHANS);

	reg = mmio_read32(dev->regs + REMOTE_IRQ_STATUS_REG);

	return reg & REMOTE_RX_IRQ(chan);
}

static void
sunxi_msgbox_handle_msg(struct device *dev, uint8_t chan, uint32_t msg)
{
	msgbox_handler *handler = get_handler(dev, chan);

	if (handler != NULL) {
		handler(dev, chan, msg);
		return;
	}

	debug("%s: %u: Unsolicited message 0x%08x", dev->name, chan, msg);
}

static void
sunxi_msgbox_irq(void *param)
{
	struct device *dev = param;
	uint32_t msg, reg;

	reg = mmio_read32(dev->regs + IRQ_STATUS_REG);
	for (uint8_t chan = 0; chan < SUNXI_MSGBOX_CHANS; ++chan) {
		if (!(reg & RX_IRQ(chan)))
			continue;
		while (sunxi_msgbox_peek_data(dev, chan)) {
			msg = mmio_read32(dev->regs + RX_MSG_DATA_REG(chan));
			sunxi_msgbox_handle_msg(dev, chan, msg);
		}
		/* Clear the pending interrupt once the FIFO is empty. */
		mmio_write32(dev->regs + IRQ_STATUS_REG, RX_IRQ(chan));
	}
}

static int
sunxi_msgbox_probe(struct device *dev)
{
	int err;

	/* Ensure a handler array was allocated. */
	assert(dev->drvdata);

	if ((err = dm_setup_clocks(dev, 1)))
		return err;

	/* Drain all messages (required to clear interrupts). */
	for (uint8_t chan = 0; chan < SUNXI_MSGBOX_CHANS; ++chan) {
		while (sunxi_msgbox_peek_data(dev, chan))
			mmio_read32(dev->regs + RX_MSG_DATA_REG(chan));
	}

	/* Disable and clear all interrupts. */
	mmio_write32(dev->regs + IRQ_EN_REG, 0);
	mmio_write32(dev->regs + IRQ_STATUS_REG, BITMASK(0, 16));

	if ((err = dm_setup_irq(dev, sunxi_msgbox_irq)))
		return err;

	return SUCCESS;
}

const struct msgbox_driver sunxi_msgbox_driver = {
	.drv = {
		.class = DM_CLASS_MSGBOX,
		.probe = sunxi_msgbox_probe,
	},
	.ops = {
		.disable    = sunxi_msgbox_disable,
		.enable     = sunxi_msgbox_enable,
		.send       = sunxi_msgbox_send,
		.tx_pending = sunxi_msgbox_tx_pending,
	},
};
