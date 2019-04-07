/*
 * Copyright © 2017-2019 The Crust Firmware Authors.
 * SPDX-License-Identifier: BSD-3-Clause OR GPL-2.0-only
 */

#ifndef DRIVERS_MSGBOX_H
#define DRIVERS_MSGBOX_H

#include <dm.h>
#include <intrusive.h>
#include <stdbool.h>
#include <stdint.h>

#define MSGBOX_OPS(dev) \
	(&container_of((dev)->drv, struct msgbox_driver, drv)->ops)

enum {
	MSGBOX_CHAN_SCPI_EL3_RX = 0,
	MSGBOX_CHAN_SCPI_EL3_TX = 1,
	MSGBOX_CHAN_SCPI_EL2_RX = 2,
	MSGBOX_CHAN_SCPI_EL2_TX = 3,
};

struct msgbox_driver_ops {
	int  (*disable)(struct device *dev, uint8_t chan);
	int  (*enable)(struct device *dev, uint8_t chan);
	bool (*last_tx_done)(struct device *dev, uint8_t chan);
	int  (*send)(struct device *dev, uint8_t chan,
	             uint32_t message);
};

struct msgbox_driver {
	const struct driver            drv;
	const struct msgbox_driver_ops ops;
};

/**
 * Disable a message channel, so it will no longer be able to receive messages.
 *
 * @param dev  The message box device.
 * @param chan The message box channel.
 */
static inline int
msgbox_disable(struct device *dev, uint8_t chan)
{
	return MSGBOX_OPS(dev)->disable(dev, chan);
}

/**
 * Enable a message box channel to receive messages.
 *
 * @param dev     The message box device.
 * @param chan    The message box channel.
 */
static inline int
msgbox_enable(struct device *dev, uint8_t chan)
{
	return MSGBOX_OPS(dev)->enable(dev, chan);
}

/**
 * Check if the last transmission on a message box channel has completed, or if
 * it is still pending. A message is pending until the reception IRQ has been
 * cleared on the remote interface.
 *
 * @param dev  The message box device.
 * @param chan The message box channel.
 */
static inline bool
msgbox_last_tx_done(struct device *dev, uint8_t chan)
{
	return MSGBOX_OPS(dev)->last_tx_done(dev, chan);
}

/**
 * Send a message via a message box device. The message box channel must have
 * previously been enabled.
 *
 * @param dev     The message box device.
 * @param chan    The channel to use within the message box.
 * @param message The message to send.
 */
static inline int
msgbox_send(struct device *dev, uint8_t chan, uint32_t message)
{
	return MSGBOX_OPS(dev)->send(dev, chan, message);
}

#endif /* DRIVERS_MSGBOX_H */
