/*
 * Copyright © 2017-2018 The Crust Firmware Authors.
 * SPDX-License-Identifier: (BSD-3-Clause OR GPL-2.0)
 */

#include <macros.S>

func panic
	l.jal	log			# Log the panic message (clobbers r9)
	l.nop
1:	l.j	1b			# Infinite loop
	l.nop
endfunc panic
