/*
 * Copyright © 2017-2018 The Crust Firmware Authors.
 * SPDX-License-Identifier: BSD-3-Clause OR GPL-2.0-only
 */

#ifndef COMMON_WALLCLOCK_H
#define COMMON_WALLCLOCK_H

#include <stdint.h>

/**
 * Read the platform's "wall clock", which is a monotonically-increasing 64-bit
 * time stamp counter running at the platform's reference clock frequency. It
 * starts counting from zero when the SoC is reset.
 */
uint64_t wallclock_read(void);

#endif /* COMMON_WALLCLOCK_H */
