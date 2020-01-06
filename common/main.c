/*
 * Copyright © 2017-2019 The Crust Firmware Authors.
 * SPDX-License-Identifier: BSD-3-Clause OR GPL-2.0-only
 */

#include <debug.h>
#include <spr.h>
#include <system.h>

noreturn void main(uint32_t exception);

noreturn void
main(uint32_t exception)
{
	if (exception) {
		error("Unhandled exception %u at %p!",
		      exception, (void *)mfspr(SPR_SYS_EPCR_INDEX(0)));
	}

	/* Cede control to the system state machine (never returns). */
	system_state_machine();
}
