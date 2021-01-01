/*
 * Copyright © 2017-2021 The Crust Firmware Authors.
 * SPDX-License-Identifier: BSD-3-Clause OR GPL-2.0-only
 */

#ifndef COUNTER_H
#define COUNTER_H

#include <stdint.h>

/**
 * Initialize the system counter.
 *
 * This function must be called before delays or timeouts can be used.
 */
void counter_init(void);

/**
 * Read the system counter.
 *
 * This counter (the OpenRISC 1000 architectural tick timer) is a 32-bit up
 * counter running at the same frequency as the CPU clock.
 */
uint32_t counter_read(void);

#endif /* COUNTER_H */
