/*
 * Copyright © 2017-2020 The Crust Firmware Authors.
 * SPDX-License-Identifier: BSD-3-Clause OR GPL-2.0-only
 */

#ifndef COMMON_TIMEOUT_H
#define COMMON_TIMEOUT_H

#include <stdbool.h>
#include <stdint.h>

#define USEC_PER_MSEC 1000U
#define USEC_PER_SEC  1000000U

/**
 * Check if a timeout has expired.
 *
 * @param timeout The timeout.
 * @return        Whether or not the timeout has expired.
 */
bool timeout_expired(uint32_t timeout);

/**
 * Set a timeout for some point in the near future.
 *
 * Timeouts must not be set for greater than approximately one minute.
 *
 * @param useconds The delay in microseconds before the timeout expires.
 * @return         An opaque number that can be passed to timeout_expired().
 */
uint32_t timeout_set(uint32_t useconds);

#endif /* COMMON_TIMEOUT_H */
