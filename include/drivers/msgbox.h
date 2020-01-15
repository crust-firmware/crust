/*
 * Copyright © 2017-2019 The Crust Firmware Authors.
 * SPDX-License-Identifier: BSD-3-Clause OR GPL-2.0-only
 */

#ifndef DRIVERS_MSGBOX_H
#define DRIVERS_MSGBOX_H

#include <device.h>
#include <stdbool.h>
#include <stdint.h>

/**
 * Acknowledge a message received on a message box channel.
 *
 * @param dev  The message box device.
 * @param chan The message box channel.
 */
void msgbox_ack_rx(const struct device *dev, uint8_t chan);

/**
 * Check if the last transmission on a message box channel has completed, or if
 * it is still pending. A message is pending until the reception IRQ has been
 * cleared on the remote interface.
 *
 * @param dev  The message box device.
 * @param chan The message box channel.
 */
bool msgbox_last_tx_done(const struct device *dev, uint8_t chan);

/**
 * Receive a message via a message box channel.
 *
 * @param dev     The message box device.
 * @param chan    The channel to use within the message box.
 * @param message The message to receive.
 */
int msgbox_receive(const struct device *dev, uint8_t chan, uint32_t *message);

/**
 * Send a message via a message box channel.
 *
 * @param dev     The message box device.
 * @param chan    The channel to use within the message box.
 * @param message The message to send.
 */
int msgbox_send(const struct device *dev, uint8_t chan, uint32_t message);

#endif /* DRIVERS_MSGBOX_H */
