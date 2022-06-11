/*
 * Copyright © 2017-2022 The Crust Firmware Authors.
 * SPDX-License-Identifier: BSD-3-Clause OR GPL-2.0-only
 */

#ifndef COUNTER_H
#define COUNTER_H

#include <stdint.h>

/**
 * Initialize the cycle counter.
 *
 * This function must be called once before attempting to read the cycle
 * counter. Note that the delay and timeout libraries use the cycle counter.
 */
void cycle_counter_init(void);

/**
 * Read the cycle counter.
 *
 * This counter (the OpenRISC 1000 architectural tick timer) is a 32-bit up
 * counter running at the same frequency as the CPU clock.
 */
uint32_t cycle_counter_read(void);

#endif /* COUNTER_H */
