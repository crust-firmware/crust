/*
 * Copyright © 2017-2020 The Crust Firmware Authors.
 * SPDX-License-Identifier: BSD-3-Clause OR GPL-2.0-only
 */

#ifndef MSGBOX_PRIVATE_H
#define MSGBOX_PRIVATE_H

#include <device.h>
#include <stdbool.h>
#include <stdint.h>

struct msgbox_driver_ops {
	void (*ack_rx)(const struct device *dev, uint8_t chan);
	bool (*last_tx_done)(const struct device *dev, uint8_t chan);
	int  (*receive)(const struct device *dev, uint8_t chan,
	                uint32_t *message);
	int  (*send)(const struct device *dev, uint8_t chan,
	             uint32_t message);
};

struct msgbox_driver {
	struct driver            drv;
	struct msgbox_driver_ops ops;
};

#endif /* MSGBOX_PRIVATE_H */
