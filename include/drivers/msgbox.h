/*
 * Copyright © 2017 Samuel Holland <samuel@sholland.org>
 * SPDX-License-Identifier: (BSD-3-Clause OR GPL-2.0)
 */

#ifndef DRIVERS_MSGBOX_H
#define DRIVERS_MSGBOX_H

#include <dm.h>
#include <stdint.h>

struct msgbox_driver_ops {
	uint32_t class;
};

#endif /* DRIVERS_MSGBOX_H */
