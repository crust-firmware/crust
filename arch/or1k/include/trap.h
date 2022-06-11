/*
 * Copyright © 2017-2022 The Crust Firmware Authors.
 * SPDX-License-Identifier: BSD-3-Clause OR GPL-2.0-only
 */

#ifndef TRAP_H
#define TRAP_H

static inline noreturn void
trap(void)
{
	asm volatile ("l.trap 0");
	unreachable();
}

#endif /* TRAP_H */
