/*
 * Copyright © 2017-2021 The Crust Firmware Authors.
 * SPDX-License-Identifier: BSD-3-Clause OR GPL-2.0-only
 */

#ifndef DRIVERS_WATCHDOG_H
#define DRIVERS_WATCHDOG_H

#include <device.h>

/**
 * Restart the watchdog. This must be called before the watchdog times out.
 *
 * @param dev The watchdog device.
 */
void watchdog_restart(const struct device *dev);

/**
 * Set the watchdog timeout. It will take effect after the next restart.
 *
 * @param dev     The watchdog device.
 * @param timeout The new watchdog timeout in clock cycles.
 */
void watchdog_set_timeout(const struct device *dev, uint32_t timeout);

#endif /* DRIVERS_WATCHDOG_H */
