/*
 * Copyright © 2017-2020 The Crust Firmware Authors.
 * SPDX-License-Identifier: BSD-3-Clause OR GPL-2.0-only
 */

#ifndef COMMON_SYSTEM_H
#define COMMON_SYSTEM_H

#include <stdbool.h>

enum {
	SYSTEM_INVALID,  /**< Uninitialized state machine (initial state). */
	SYSTEM_ACTIVE,   /**< ARM CPUs are running firmware or an OS. */
	SYSTEM_SUSPEND,  /**< Transition from active to inactive. */
	SYSTEM_INACTIVE, /**< ARM CPUs are not running; RAM is preserved. */
	SYSTEM_RESUME,   /**< Transition from inactive to active. */
	SYSTEM_SHUTDOWN, /**< Transition from active to off. */
	SYSTEM_OFF,      /**< ARM CPUs are not running; RAM is invalid. */
	SYSTEM_REBOOT,   /**< Board-level reset is in progress. */
	SYSTEM_RESET,    /**< SoC-level reset is in progress (final state). */
};

/**
 * Get the current system state.
 */
uint8_t get_system_state(void);

/**
 * Check if the system is in a state where it can be woken up.
 */
bool system_can_wake(void) ATTRIBUTE(pure);

/**
 * Check if the system is in a state where the main CPUs are executing.
 */
bool system_is_running(void) ATTRIBUTE(pure);

/**
 * Perform system state management.
 *
 * This is the main loop of the firmware, and never returns.
 */
noreturn void system_state_machine(void);

/**
 * Reboot the board, including the SoC and external peripherals.
 *
 * May be called at any time.
 */
void system_reboot(void);

/**
 * Reset the SoC, including all CPUs and internal peripherals.
 *
 * May be called at any time.
 */
void system_reset(void);

/**
 * Shut down the SoC, and turn off all possible power domains.
 *
 * Must only be called while the system is active.
 */
void system_shutdown(void);

/**
 * Suspend the SoC, and turn off all non-wakeup power domains.
 *
 * Must only be called while the system is active.
 */
void system_suspend(void);

/**
 * Wake up the SoC, and turn on previously-disabled power domains.
 *
 * Should only be called while the system is inactive (suspended).
 */
void system_wakeup(void);

#endif /* COMMON_SYSTEM_H */
