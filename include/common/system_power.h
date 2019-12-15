/*
 * Copyright © 2017-2019 The Crust Firmware Authors.
 * SPDX-License-Identifier: BSD-3-Clause OR GPL-2.0-only
 */

#ifndef COMMON_SYSTEM_POWER_H
#define COMMON_SYSTEM_POWER_H

#include <compiler.h>
#include <stdbool.h>

enum {
	SYSTEM_ACTIVE,
	SYSTEM_SUSPEND,
	SYSTEM_INACTIVE,
	SYSTEM_RESUME,
	SYSTEM_SHUTDOWN,
	SYSTEM_OFF,
	SYSTEM_RESET,
};

/**
 * Check if the system is in a state where it can be woken up.
 */
bool system_can_wake(void) ATTRIBUTE(pure);

/**
 * Check if the system is in a state where the main CPUs are executing.
 */
bool system_is_running(void) ATTRIBUTE(pure);

/**
 * Perform system state transitions.
 */
void system_state_machine(void);

/**
 * Reset the SoC, including all CPUs and internal peripherals.
 */
void system_reset(void);

/**
 * Shut down the SoC, and turn off all possible external voltage domains.
 */
void system_shutdown(void);

/**
 * Suspend the SoC, and turn off all possible external voltage domains.
 */
void system_suspend(void);

/**
 * Wake up the SoC, and turn on necessary external voltage domains.
 */
void system_wakeup(void);

#endif /* COMMON_SYSTEM_POWER_H */
