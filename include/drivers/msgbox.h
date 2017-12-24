/*
 * Copyright © 2017 Samuel Holland <samuel@sholland.org>
 * SPDX-License-Identifier: (BSD-3-Clause OR GPL-2.0)
 */

#ifndef DRIVERS_MSGBOX_H
#define DRIVERS_MSGBOX_H

#include <dm.h>
#include <stdint.h>

#define MSGBOX_OPS(ops) ((struct msgbox_driver_ops *)(ops))

typedef void (*msg_handler)(struct device *dev, uint8_t chan, uint32_t msg);

struct msgbox_driver_ops {
	int (*register_handler)(struct device *dev, uint8_t chan,
	                        msg_handler handler);
	int (*send_msg)(struct device *dev, uint8_t chan, uint32_t msg);
	int (*unregister_handler)(struct device *dev, uint8_t chan);
};

static inline int
msgbox_register_handler(struct device *dev, uint8_t chan, msg_handler handler)
{
	return MSGBOX_OPS(dev->drv->ops)->register_handler(dev, chan, handler);
}

static inline int
msgbox_send_msg(struct device *dev, uint8_t chan, uint32_t msg)
{
	return MSGBOX_OPS(dev->drv->ops)->send_msg(dev, chan, msg);
}

static inline int
msgbox_unregister_handler(struct device *dev, uint8_t chan)
{
	return MSGBOX_OPS(dev->drv->ops)->unregister_handler(dev, chan);
}

#endif /* DRIVERS_MSGBOX_H */
