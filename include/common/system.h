/*
 * Copyright © 2017-2019 The Crust Firmware Authors.
 * SPDX-License-Identifier: BSD-3-Clause OR GPL-2.0-only
 */

#ifndef COMMON_SYSTEM_H
#define COMMON_SYSTEM_H

#include <stdbool.h>

enum {
	SYSTEM_ACTIVE,   /**< ARM CPUs are running firmware or an OS. */
	SYSTEM_SUSPEND,  /**< Transition from active to inactive. */
	SYSTEM_INACTIVE, /**< ARM CPUs are not running; RAM is preserved. */
	SYSTEM_RESUME,   /**< Transition from inactive to active. */
	SYSTEM_SHUTDOWN, /**< Transition from active to off. */
	SYSTEM_OFF,      /**< ARM CPUs are not running; RAM is invalid. */
	SYSTEM_RESET,    /**< System reset is in progress (final state). */
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
 * Perform system state machine initialization.
 */
void system_state_init(void);

/**
 * Perform system state transitions.
 */
void system_state_machine(void);

/**
 * Reset the SoC, including all CPUs and internal peripherals.
 *
 * Should not be called durring a system state transition.
 */
void system_reset(void);

/**
 * Shut down the SoC, and turn off all possible power domains.
 *
 * Should not be called durring a system state transition.
 */
void system_shutdown(void);

/**
 * Suspend the SoC, and turn off all non-wakeup power domains.
 *
 * Should not be called durring a system state transition.
 */
void system_suspend(void);

/**
 * Wake up the SoC, and turn on previously-disabled power domains.
 *
 * Should not be called durring a system state transition.
 */
void system_wakeup(void);

#endif /* COMMON_SYSTEM_H */
