/*
 * Copyright © 2017-2019 The Crust Firmware Authors.
 * SPDX-License-Identifier: BSD-3-Clause OR GPL-2.0-only
 */

#ifndef COMMON_MONITORING_H
#define COMMON_MONITORING_H

#include <stdbool.h>

/**
 * Poll the temperature sensors, throttling the system as necessary. This
 * function should be called once per second.
 */
void poll_sensors(void);

/**
 * Check if the system is being throttled due to overtemperature.
 */
bool system_is_throttled(void) __pure;

#endif /* COMMON_MONITORING_H */
