/*
 * Copyright © 2017 Samuel Holland <samuel@sholland.org>
 * SPDX-License-Identifier: (BSD-3-Clause OR GPL-2.0)
 */

#include <debug.h>
#include <dm.h>
#include <error.h>
#include <mmio.h>
#include <stddef.h>
#include <util.h>
#include <drivers/clock.h>
#include <drivers/irqchip.h>
#include <drivers/msgbox.h>

/*
 * The message box hardware provides 8 unidirectional channels. As the mailbox
 * framework expects them to be bidirectional, create virtual channels out of
 * pairs of opposite-direction hardware channels. The first channel in each
 * pair
 * is set up for AP->SCP communication, and the second channel is set up for
 * SCP->AP transmission.
 */
#define NUM_CHANS               4

#define CTRL_REG0               (0x0000)
#define CTRL_REG1               (0x0004)

/* These macros take a virtual channel number. */
#define IRQ_EN_REG              (0x0040)
#define IRQ_STATUS_REG          (0x0050)
#define XMIT_IRQ(n)             BIT(3 + 4 * (n))
#define RECV_IRQ(n)             BIT(0 + 4 * (n))

#define XMIT_FIFO_STATUS_REG(n) (0x0104 + 0x8 * (n))
#define RECV_FIFO_STATUS_REG(n) (0x0100 + 0x8 * (n))
#define FIFO_STATUS_MASK        BIT(0)

#define XMIT_MSG_STATUS_REG(n)  (0x0144 + 0x8 * (n))
#define RECV_MSG_STATUS_REG(n)  (0x0140 + 0x8 * (n))
#define MSG_STATUS_MASK         BITMASK(0, 3)

#define XMIT_MSG_DATA_REG(n)    (0x0184 + 0x8 * (n))
#define RECV_MSG_DATA_REG(n)    (0x0180 + 0x8 * (n))

static msg_handler handlers[NUM_CHANS];

static void
sunxi_msgbox_handle_msg(struct device *dev, uint32_t chan)
{
	uint32_t msg = mmio_read32(dev->address + RECV_MSG_DATA_REG(chan));

	if (handlers[chan])
		handlers[chan](dev, chan, msg);
	else
		debug("unsolicited message %08x in channel %d", msg, chan);
}

static void
sunxi_msgbox_irq(struct device *dev)
{
	uint32_t reg = mmio_read32(dev->address + IRQ_STATUS_REG);

	for (uint32_t chan = 0; chan < NUM_CHANS; ++chan) {
		if (!(reg & RECV_IRQ(chan)))
			continue;
		while (mmio_read32(dev->address + RECV_MSG_STATUS_REG(chan)))
			sunxi_msgbox_handle_msg(dev, chan);
	}

	/* Clear all processed pending interrupts. */
	mmio_write32(dev->address + IRQ_STATUS_REG, reg);
}

static int
sunxi_msgbox_register_handler(struct device *dev, uint8_t chan,
                              msg_handler handler)
{
	uint32_t reg;

	assert(chan < NUM_CHANS);

	if (handlers[chan])
		return EEXIST;
	handlers[chan] = handler;

	reg = mmio_read32(dev->address + IRQ_EN_REG);
	mmio_write32(dev->address + IRQ_EN_REG, reg | RECV_IRQ(chan));

	return 0;
}

static int
sunxi_msgbox_probe(struct device *dev)
{
	int err;

	if ((err = clock_enable(dev)))
		return err;

	/* Disable and clear all IRQs. */
	mmio_write32(dev->address + IRQ_EN_REG, 0);
	mmio_write32(dev->address + IRQ_STATUS_REG, BITMASK(0, 16));

	if ((err = irqchip_register_irq(dev, sunxi_msgbox_irq)))
		return err;

	return SUCCESS;
}

static int
sunxi_msgbox_send_msg(struct device *dev, uint8_t chan, uint32_t msg)
{
	mmio_write32(dev->address + XMIT_MSG_DATA_REG(chan), msg);

	return 0;
}

static int
sunxi_msgbox_unregister_handler(struct device *dev, uint8_t chan)
{
	uint32_t reg;

	assert(handlers[chan]);

	reg = mmio_read32(dev->address + IRQ_EN_REG);
	mmio_write32(dev->address + IRQ_EN_REG, reg & ~RECV_IRQ(chan));

	handlers[chan] = NULL;

	return 0;
}

static struct msgbox_driver_ops sunxi_msgbox_driver_ops = {
	.class              = DM_CLASS_MSGBOX,
	.register_handler   = sunxi_msgbox_register_handler,
	.send_msg           = sunxi_msgbox_send_msg,
	.unregister_handler = sunxi_msgbox_unregister_handler,
};

struct driver sunxi_msgbox_driver = {
	.name  = "sunxi-msgbox",
	.probe = sunxi_msgbox_probe,
	.ops   = &sunxi_msgbox_driver_ops,
};
