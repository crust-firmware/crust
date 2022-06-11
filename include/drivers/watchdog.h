/*
 * Copyright © 2017-2022 The Crust Firmware Authors.
 * SPDX-License-Identifier: BSD-3-Clause OR GPL-2.0-only
 */

#ifndef DRIVERS_WATCHDOG_H
#define DRIVERS_WATCHDOG_H

#include <device.h>

/**
 * Get a reference to an available watchdog device.
 *
 * @return A reference to a watchdog device.
 */
const struct device *watchdog_get(void);

/**
 * Use the watchdog to reset the system as soon as possible.
 *
 * @param dev The watchdog device.
 */
void watchdog_reset_system(const struct device *dev);

/**
 * Restart the watchdog. This must be called before the watchdog times out.
 *
 * @param dev The watchdog device.
 */
void watchdog_restart(const struct device *dev);

#endif /* DRIVERS_WATCHDOG_H */
