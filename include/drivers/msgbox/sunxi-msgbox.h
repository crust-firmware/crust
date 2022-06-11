/*
 * Copyright © 2017-2022 The Crust Firmware Authors.
 * SPDX-License-Identifier: BSD-3-Clause OR GPL-2.0-only
 */

#ifndef DRIVERS_MSGBOX_SUNXI_MSGBOX_H
#define DRIVERS_MSGBOX_SUNXI_MSGBOX_H

#include <msgbox.h>
#include <simple_device.h>

/* The message box hardware provides 8 unidirectional channels. */
#define SUNXI_MSGBOX_CHANS 8

extern const struct simple_device msgbox;

#endif /* DRIVERS_MSGBOX_SUNXI_MSGBOX_H */
