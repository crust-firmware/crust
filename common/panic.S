/*
 * Copyright © 2017-2019 The Crust Firmware Authors.
 * SPDX-License-Identifier: BSD-3-Clause OR GPL-2.0-only
 */

#include <macros.S>
#include <arch/spr.h>

func panic
	l.jal	log			# Log the panic message (clobbers r9)
	l.nop
	l.trap	0
endfunc panic
