/*
 * Copyright © 2017-2022 The Crust Firmware Authors.
 * SPDX-License-Identifier: BSD-3-Clause OR GPL-2.0-only
 */

#include <delay.h>
#include <stdint.h>
#include <timeout.h>

void
udelay(uint32_t useconds)
{
	uint32_t timeout = timeout_set(useconds);

	while (!timeout_expired(timeout)) {
		/* Do nothing. */
	}
}
