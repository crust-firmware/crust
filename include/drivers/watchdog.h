/*
 * Copyright © 2017-2020 The Crust Firmware Authors.
 * SPDX-License-Identifier: BSD-3-Clause OR GPL-2.0-only
 */

#ifndef DRIVERS_WATCHDOG_H
#define DRIVERS_WATCHDOG_H

#include <device.h>
#include <stdint.h>

/**
 * Disable the watchdog.
 *
 * @param dev The watchdog device.
 */
void watchdog_disable(const struct device *dev);

/**
 * Enable and restart the watchdog.
 *
 * @param dev     The watchdog device.
 * @param timeout The watchdog timeout in clock cycles.
 */
int watchdog_enable(const struct device *dev, uint32_t timeout);

/**
 * Restart the watchdog. This must be called before the watchdog times out.
 *
 * @param dev The watchdog device.
 */
void watchdog_restart(const struct device *dev);

#endif /* DRIVERS_WATCHDOG_H */
