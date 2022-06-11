/*
 * Copyright © 2017-2022 The Crust Firmware Authors.
 * SPDX-License-Identifier: BSD-3-Clause OR GPL-2.0-only
 */

#ifndef DRIVERS_WATCHDOG_SUNXI_TWD_H
#define DRIVERS_WATCHDOG_SUNXI_TWD_H

#include <simple_device.h>
#include <stdint.h>
#include <watchdog.h>

extern const struct simple_device r_twd;

/**
 * Read the low 32 bits of the trusted watchdog counter.
 *
 * This counter increments at 24MHz while the watchdog is enabled.
 */
uint32_t r_twd_counter_read(void);

#endif /* DRIVERS_WATCHDOG_SUNXI_TWD_H */
