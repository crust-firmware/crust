/*
 * Copyright © 2017-2019 The Crust Firmware Authors.
 * SPDX-License-Identifier: BSD-3-Clause OR GPL-2.0-only
 */

#ifndef DRIVERS_MSGBOX_SUNXI_MSGBOX_H
#define DRIVERS_MSGBOX_SUNXI_MSGBOX_H

#include <msgbox.h>

/* The message box hardware provides 8 unidirectional channels. */
#define SUNXI_MSGBOX_CHANS 8

#define SUNXI_MSGBOX_DRVDATA \
	(uintptr_t)&(msgbox_handler *[SUNXI_MSGBOX_CHANS])

extern struct device msgbox;

#endif /* DRIVERS_MSGBOX_SUNXI_MSGBOX_H */
