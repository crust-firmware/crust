/*
 * Copyright © 2017-2019 The Crust Firmware Authors.
 * SPDX-License-Identifier: BSD-3-Clause OR GPL-2.0-only
 */

#ifndef DRIVERS_MSGBOX_SUNXI_MSGBOX_H
#define DRIVERS_MSGBOX_SUNXI_MSGBOX_H

#include <clock.h>
#include <msgbox.h>

/* The message box hardware provides 8 unidirectional channels. */
#define SUNXI_MSGBOX_CHANS 8

struct sunxi_msgbox {
	struct device       dev;
	struct clock_handle clock;
	uintptr_t           regs;
};

extern const struct sunxi_msgbox msgbox;

#endif /* DRIVERS_MSGBOX_SUNXI_MSGBOX_H */
