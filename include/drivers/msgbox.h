/*
 * Copyright © 2017-2018 The Crust Firmware Authors.
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

typedef void msgbox_handler (struct device *dev, uint8_t, uint32_t);

struct msgbox_driver_ops {
	int  (*disable)(struct device *dev, uint8_t chan);
	int  (*enable)(struct device *dev, uint8_t chan,
	               msgbox_handler *handler);
	int  (*send)(struct device *dev, uint8_t chan,
	             uint32_t message);
	bool (*tx_pending)(struct device *dev, uint8_t chan);
};

struct msgbox_driver {
	const struct driver            drv;
	const struct msgbox_driver_ops ops;
};

/**
 * Disable a message channel, so it will no longer be able to send or receive
 * messages.
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
 * Enable a message box channel to send and receive messages. The provided
 * handler is called for each incoming message.
 *
 * @param dev     The message box device.
 * @param chan    The message box channel.
 * @param handler The callback function to register.
 */
static inline int
msgbox_enable(struct device *dev, uint8_t chan, msgbox_handler *handler)
{
	return MSGBOX_OPS(dev)->enable(dev, chan, handler);
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

/**
 * Check if a previous transmission on a message box channel is still pending.
 * A message is pending until the reception IRQ has been cleared on the remote
 * interface.
 *
 * @param dev  The message box device.
 * @param chan The message box channel.
 */
static inline bool
msgbox_tx_pending(struct device *dev, uint8_t chan)
{
	return MSGBOX_OPS(dev)->tx_pending(dev, chan);
}

#endif /* DRIVERS_MSGBOX_H */
