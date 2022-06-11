/*
 * Copyright © 2017-2022 The Crust Firmware Authors.
 * SPDX-License-Identifier: BSD-3-Clause OR GPL-2.0-only
 */

#ifndef DRIVERS_COUNTER_H
#define DRIVERS_COUNTER_H

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
 * This counter is at least 32 bits wide and runs at the CPU clock frequency.
 */
uint32_t cycle_counter_read(void);

/**
 * Read the system counter.
 *
 * This counter is at least 32 bits wide and runs at the platform's reference
 * clock frequency (usually 24 MHz). Note that this clock may be unavailable
 * while the system is off or asleep.
 */
uint32_t system_counter_read(void);

#endif /* DRIVERS_COUNTER_H */
