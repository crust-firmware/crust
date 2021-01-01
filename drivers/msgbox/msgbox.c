/*
 * Copyright © 2017-2021 The Crust Firmware Authors.
 * SPDX-License-Identifier: BSD-3-Clause OR GPL-2.0-only
 */

#include <device.h>
#include <intrusive.h>
#include <msgbox.h>
#include <stdbool.h>
#include <stdint.h>

#include "msgbox.h"

/**
 * Get the ops for the msgbox controller device.
 */
static inline const struct msgbox_driver_ops *
msgbox_ops_for(const struct device *dev)
{
	const struct msgbox_driver *drv =
		container_of(dev->drv, const struct msgbox_driver, drv);

	return &drv->ops;
}

void
msgbox_ack_rx(const struct device *dev, uint8_t chan)
{
	msgbox_ops_for(dev)->ack_rx(dev, chan);
}

bool
msgbox_last_tx_done(const struct device *dev, uint8_t chan)
{
	return msgbox_ops_for(dev)->last_tx_done(dev, chan);
}

int
msgbox_receive(const struct device *dev, uint8_t chan, uint32_t *message)
{
	return msgbox_ops_for(dev)->receive(dev, chan, message);
}

int
msgbox_send(const struct device *dev, uint8_t chan, uint32_t message)
{
	return msgbox_ops_for(dev)->send(dev, chan, message);
}
