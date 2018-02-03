/*
 * Copyright © 2017-2018 The Crust Firmware Authors.
 * SPDX-License-Identifier: (BSD-3-Clause OR GPL-2.0)
 */

#ifndef CONSOLE_H
#define CONSOLE_H

#include <stdint.h>

void console_init(uintptr_t base);
void console_putc(uint32_t c);

#endif /* CONSOLE_H */
