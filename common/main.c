/*
 * Copyright © 2017-2019 The Crust Firmware Authors.
 * SPDX-License-Identifier: BSD-3-Clause OR GPL-2.0-only
 */

#include <compiler.h>
#include <stdbool.h>
#include <msgbox/sunxi-msgbox.h>

noreturn void main(void);

noreturn void
main(void)
{
	msgbox.drv->probe(&msgbox);

	while (true)
		msgbox.drv->poll(&msgbox);
}
