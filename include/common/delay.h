/*
 * Copyright © 2017-2018 The Crust Firmware Authors.
 * SPDX-License-Identifier: (BSD-3-Clause OR GPL-2.0)
 */

#ifndef DELAY_H
#define DELAY_H

#include <stdint.h>

/**
 * Spin (do nothing) for at least the given number of reference clock (24MHz)
 * cycles.
 *
 * @param The number of cycles to delay for.
 */
void delay_cycles(uint32_t cycles);

#endif /* DELAY_H */
