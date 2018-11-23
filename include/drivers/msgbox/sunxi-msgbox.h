/*
 * Copyright © 2017-2018 The Crust Firmware Authors.
 * SPDX-License-Identifier: BSD-3-Clause OR GPL-2.0-only
 */

#ifndef DRIVERS_MSGBOX_SUNXI_MSGBOX_H
#define DRIVERS_MSGBOX_SUNXI_MSGBOX_H

#include <msgbox.h>

/*
 * The message box hardware provides 8 unidirectional channels. As the mailbox
 * framework expects them to be bidirectional, create virtual channels out of
 * pairs of opposite-direction hardware channels. The first channel in each
 * pair is set up for AP->SCP communication, and the second channel is set up
 * for SCP->AP transmission.
 */
#define SUNXI_MSGBOX_CHANS 4

#define SUNXI_MSGBOX_DRVDATA \
	(uintptr_t)&(msgbox_handler *[SUNXI_MSGBOX_CHANS])

extern const struct msgbox_driver sunxi_msgbox_driver;

#endif /* DRIVERS_MSGBOX_SUNXI_MSGBOX_H */
