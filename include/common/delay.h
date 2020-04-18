/*
 * Copyright © 2017-2020 The Crust Firmware Authors.
 * SPDX-License-Identifier: BSD-3-Clause OR GPL-2.0-only
 */

#ifndef COMMON_DELAY_H
#define COMMON_DELAY_H

#include <stdint.h>

/**
 * Spin (do nothing) for at least the given number of microseconds.
 *
 * @param useconds The number of microseconds to delay for.
 */
void udelay(uint32_t useconds);

#endif /* COMMON_DELAY_H */
