/*
 * Copyright © 2017-2018 The Crust Firmware Authors.
 * SPDX-License-Identifier: (BSD-3-Clause OR GPL-2.0)
 */

#ifndef DRIVERS_MSGBOX_H
#define DRIVERS_MSGBOX_H

#include <dm.h>
#include <stdbool.h>
#include <stdint.h>

#define MSGBOX_OPS(dev) ((struct msgbox_driver_ops *)((dev)->drv->ops))

typedef void (*msgbox_handler)(struct device *dev, uint8_t chan, uint32_t msg);

struct msgbox_driver_ops {
	int  (*register_handler)(struct device *dev, uint8_t chan,
	                         msgbox_handler handler);
	int  (*send_msg)(struct device *dev, uint8_t chan, uint32_t msg);
	bool (*tx_pending)(struct device *dev, uint8_t chan);
	int  (*unregister_handler)(struct device *dev, uint8_t chan);
};

static inline int
msgbox_register_handler(struct device *dev, uint8_t chan,
                        msgbox_handler handler)
{
	return MSGBOX_OPS(dev)->register_handler(dev, chan, handler);
}

static inline int
msgbox_send_msg(struct device *dev, uint8_t chan, uint32_t msg)
{
	return MSGBOX_OPS(dev)->send_msg(dev, chan, msg);
}

static inline bool
msgbox_tx_pending(struct device *dev, uint8_t chan)
{
	return MSGBOX_OPS(dev)->tx_pending(dev, chan);
}

static inline int
msgbox_unregister_handler(struct device *dev, uint8_t chan)
{
	return MSGBOX_OPS(dev)->unregister_handler(dev, chan);
}

#endif /* DRIVERS_MSGBOX_H */
