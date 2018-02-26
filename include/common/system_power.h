/*
 * Copyright © 2017-2018 The Crust Firmware Authors.
 * SPDX-License-Identifier: (BSD-3-Clause OR GPL-2.0)
 */

#ifndef COMMON_SYSTEM_POWER_H
#define COMMON_SYSTEM_POWER_H

#include <compiler.h>

/**
 * Reset the SoC, including all CPU cores and internal peripheral devices.
 */
noreturn void system_reset(void);

#endif /* COMMON_SYSTEM_POWER_H */
