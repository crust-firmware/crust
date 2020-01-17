/*
 * Copyright © 2017-2020 The Crust Firmware Authors.
 * SPDX-License-Identifier: BSD-3-Clause OR GPL-2.0-only
 */

#ifndef DRIVERS_COUNTER_H
#define DRIVERS_COUNTER_H

#include <stdint.h>

/**
 * Read the platform's counter, which is a monotonically-increasing 64-bit
 * time stamp counter running at the platform's reference clock frequency. It
 * starts counting from zero when the SoC is reset.
 */
uint64_t counter_read(void);

#endif /* DRIVERS_COUNTER_H */
