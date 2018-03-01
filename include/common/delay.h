/*
 * Copyright © 2017-2018 The Crust Firmware Authors.
 * SPDX-License-Identifier: (BSD-3-Clause OR GPL-2.0)
 */

#ifndef DELAY_H
#define DELAY_H

#include <stdint.h>

/**
 * Spin (do nothing) for at least the given number of microseconds.
 *
 * @param The number of microseconds to delay for.
 */
void udelay(uint32_t useconds);

#endif /* DELAY_H */
