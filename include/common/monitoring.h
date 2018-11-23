/*
 * Copyright © 2017-2018 The Crust Firmware Authors.
 * SPDX-License-Identifier: BSD-3-Clause OR GPL-2.0-only
 */

#ifndef COMMON_MONITORING_H
#define COMMON_MONITORING_H

#include <stdbool.h>

/**
 * Start monitoring temperature sensors, throttling the system as necessary.
 */
void start_monitoring(void);

/**
 * Check if the system is being throttled due to overtemperature.
 */
bool system_is_throttled(void) __pure;

#endif /* COMMON_MONITORING_H */
