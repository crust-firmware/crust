/*
 * Copyright © 2017-2021 The Crust Firmware Authors.
 * SPDX-License-Identifier: BSD-3-Clause OR GPL-2.0-only
 */

#include <debug.h>
#include <exception.h>
#include <spr.h>

void
report_exception(uint32_t exception)
{
	if (!exception)
		return;

	error("Exception %u at %p!",
	      exception, (void *)mfspr(SPR_SYS_EPCR_ADDR(0)));
}
