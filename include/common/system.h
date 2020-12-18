/*
 * Copyright © 2017-2020 The Crust Firmware Authors.
 * SPDX-License-Identifier: BSD-3-Clause OR GPL-2.0-only
 */

#ifndef COMMON_SYSTEM_H
#define COMMON_SYSTEM_H

#include <stdint.h>

/**
 * Perform system state management.
 *
 * This is the main loop of the firmware, which never returns.
 *
 * @param exception Exception information provided by startup assembly code.
 */
noreturn void system_state_machine(uint32_t exception);

/**
 * Reboot the board, including the SoC and external peripherals.
 *
 * Must only be called while the system is awake.
 */
void system_reboot(void);

/**
 * Reset the SoC, including all CPUs and internal peripherals.
 *
 * Must only be called while the system is awake.
 */
void system_reset(void);

/**
 * Shut down the SoC, and turn off all possible power domains.
 *
 * Must only be called while the system is awake.
 */
void system_shutdown(void);

/**
 * Suspend the SoC, and turn off all non-wakeup power domains.
 *
 * Must only be called while the system is awake.
 */
void system_suspend(void);

/**
 * Wake the system.
 *
 * Transition to a state where the rich OS is awake and running, by resetting
 * the SoC or the entire board if necessary.
 *
 * May be called at any time.
 */
void system_wake(void);

#endif /* COMMON_SYSTEM_H */
