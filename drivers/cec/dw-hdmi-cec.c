/*
 * Copyright © 2021-2022 The Crust Firmware Authors.
 * SPDX-License-Identifier: BSD-3-Clause OR GPL-2.0-only
 */

#include <error.h>
#include <intrusive.h>
#include <mmio.h>
#include <util.h>
#include <cec/dw-hdmi-cec.h>
#include <clock/ccu.h>
#include <platform/devices.h>

#define CEC_STAT         0x0106
#define CEC_MUTE         0x0186
#define IH_MUTE          0x01ff
#define CEC_CTRL         0x7d00
#define CEC_MASK         0x7d02
#define CEC_POL          0x7d03
#define CEC_ADDRL        0x7d05
#define CEC_ADDRH        0x7d06
#define CEC_TX_CNT       0x7d07
#define CEC_RX_CNT       0x7d08
#define CEC_TX_DATA      0x7d10
#define CEC_RX_DATA      0x7d20
#define CEC_LOCK         0x7d30
#define CEC_WKUPCTRL     0x7d31

#define IRQ_WAKEUP       BIT(6)
#define IRQ_ALL          0xff

#define IH_MUTE_ALL      0x03

#define CEC_CTRL_STANDBY BIT(4)

#define CEC_LOCK_RELEASE 0x00

/* Set Stream Path */
#define CEC_WKUP_MSG_86  BIT(7)
/* Active Source */
#define CEC_WKUP_MSG_82  BIT(6)
/* System Audio Mode Request */
#define CEC_WKUP_MSG_70  BIT(5)
/* User Control Pressed */
#define CEC_WKUP_MSG_44  BIT(4)
/* Deck Control */
#define CEC_WKUP_MSG_42  BIT(3)
/* Play */
#define CEC_WKUP_MSG_41  BIT(2)
/* Text View On */
#define CEC_WKUP_MSG_0d  BIT(1)
/* Image View On */
#define CEC_WKUP_MSG_04  BIT(0)

#define CEC_WKUP_MSG_ALL (CEC_WKUP_MSG_86 | CEC_WKUP_MSG_82 | \
	                  CEC_WKUP_MSG_70 | CEC_WKUP_MSG_44 | \
	                  CEC_WKUP_MSG_42 | CEC_WKUP_MSG_41 | \
	                  CEC_WKUP_MSG_0d | CEC_WKUP_MSG_04)

struct dw_hdmi_cec_state {
	struct device_state ds;
	uint8_t             stash[4];
};

static inline const struct dw_hdmi_cec *
to_dw_hdmi_cec(const struct device *dev)
{
	return container_of(dev, const struct dw_hdmi_cec, dev);
}

static inline struct dw_hdmi_cec_state *
dw_hdmi_cec_state_for(const struct device *dev)
{
	return container_of(dev->state, struct dw_hdmi_cec_state, ds);
}

uint32_t
dw_hdmi_cec_poll(const struct device *dev)
{
	const struct dw_hdmi_cec *self = to_dw_hdmi_cec(dev);
	uint8_t stat;

	stat = mmio_read_8(self->regs + CEC_STAT);
	mmio_write_8(self->regs + CEC_STAT, stat);

	return stat & IRQ_WAKEUP;
}

static int
dw_hdmi_cec_probe(const struct device *dev)
{
	const struct dw_hdmi_cec *self  = to_dw_hdmi_cec(dev);
	struct dw_hdmi_cec_state *state = dw_hdmi_cec_state_for(dev);

	clock_get(&self->bus_clock);

	state->stash[0] = mmio_read_8(self->regs + CEC_MUTE);
	state->stash[1] = mmio_read_8(self->regs + CEC_MASK);
	state->stash[2] = mmio_read_8(self->regs + CEC_POL);
	state->stash[3] = mmio_read_8(self->regs + IH_MUTE);

	/* Mute all interrupts */
	mmio_write_8(self->regs + IH_MUTE, IH_MUTE_ALL);

	/* Configure CEC wake up sources */
	mmio_write_8(self->regs + CEC_WKUPCTRL, CEC_WKUP_MSG_ALL);

	/* Allow only wakeup interrupt on posedge */
	mmio_write_8(self->regs + CEC_POL, IRQ_WAKEUP);
	mmio_write_8(self->regs + CEC_MUTE, (uint8_t) ~IRQ_WAKEUP);
	mmio_write_8(self->regs + CEC_MASK, (uint8_t) ~IRQ_WAKEUP);

	/* Clear any pending interrupt */
	mmio_write_8(self->regs + CEC_STAT, IRQ_ALL);

	/* Release CEC message in RX buffer (if any) */
	mmio_write_8(self->regs + CEC_LOCK, CEC_LOCK_RELEASE);

	/*
	 * Put CEC controller in automatic mode. It will NACK all messages
	 * except those, which are allowed in CEC_WKUPCTRL register.
	 */
	mmio_write_8(self->regs + CEC_CTRL, CEC_CTRL_STANDBY);

	/* Unmute interrupts */
	mmio_write_8(self->regs + IH_MUTE, 0x00);

	return SUCCESS;
}

static void
dw_hdmi_cec_release(const struct device *dev)
{
	const struct dw_hdmi_cec *self  = to_dw_hdmi_cec(dev);
	struct dw_hdmi_cec_state *state = dw_hdmi_cec_state_for(dev);

	/* Mute all interrupts */
	mmio_write_8(self->regs + IH_MUTE, IH_MUTE_ALL);

	/* Clear wake up sources */
	mmio_write_8(self->regs + CEC_WKUPCTRL, 0x00);

	/* Put controller out of automatic mode */
	mmio_write_8(self->regs + CEC_CTRL, 0x00);

	/* Clear any pending interrupt */
	mmio_write_8(self->regs + CEC_STAT, IRQ_ALL);

	/* Release CEC message in RX buffer (if any) */
	mmio_write_8(self->regs + CEC_LOCK, CEC_LOCK_RELEASE);

	/* restore registers */
	mmio_write_8(self->regs + CEC_POL, state->stash[2]);
	mmio_write_8(self->regs + CEC_MASK, state->stash[1]);
	mmio_write_8(self->regs + CEC_MUTE, state->stash[0]);

	/* Restoring IH_MUTE must be last, since it enables interrutps */
	mmio_write_8(self->regs + IH_MUTE, state->stash[3]);

	clock_put(&self->bus_clock);
}

static const struct driver dw_hdmi_cec_driver = {
	.probe   = dw_hdmi_cec_probe,
	.release = dw_hdmi_cec_release,
};

const struct dw_hdmi_cec hdmi_cec = {
	.dev = {
		.name  = "DW HDMI CEC",
		.drv   = &dw_hdmi_cec_driver,
		.state = &(struct dw_hdmi_cec_state) { { 0 } }.ds,
	},
	.bus_clock = { .dev = &r_ccu.dev, .id = CLK_OSC24M },
	.regs      = DEV_HDMI,
};
