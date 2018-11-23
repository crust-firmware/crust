/*
 * Copyright © 2017-2018 The Crust Firmware Authors.
 * SPDX-License-Identifier: BSD-3-Clause OR GPL-2.0-only
 */

#ifndef COMMON_SYSTEM_POWER_H
#define COMMON_SYSTEM_POWER_H

#include <compiler.h>
#include <stdbool.h>

/**
 * Possible system power states, matching those defined in the SCPI protocol.
 */
enum {
	SYSTEM_POWER_STATE_SHUTDOWN = 0,
	SYSTEM_POWER_STATE_REBOOT   = 1,
	SYSTEM_POWER_STATE_RESET    = 2,
};

/**
 * Check if the system is in a simulated off state.
 */
bool system_is_off(void) __pure;

/**
 * Check if the system is suspended.
 */
bool system_is_suspended(void) __pure;

/**
 * Reset the SoC, including all CPU cores and internal peripheral devices.
 */
noreturn void system_reset(void);

/**
 * Shutdown the SoC, turn off all voltage domains, and shutdown the PMIC.
 */
void system_shutdown(void);

/**
 * Suspend the SoC and external voltage domains.
 */
void system_suspend(void);

/**
 * Wake up the SoC and external voltage domains, and resume CSS execution.
 */
void system_wakeup(void);

#endif /* COMMON_SYSTEM_POWER_H */
