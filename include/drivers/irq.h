/*
 * Copyright © 2017-2019 The Crust Firmware Authors.
 * SPDX-License-Identifier: BSD-3-Clause OR GPL-2.0-only
 */

#ifndef DRIVERS_IRQ_H
#define DRIVERS_IRQ_H

#include <stdbool.h>
#include <stdint.h>

/**
 * Determine if a specific IRQ is enabled.
 */
bool irq_is_enabled(uint32_t irq);

/**
 * Determine if a specific IRQ is pending (it may or may not be enabled).
 */
bool irq_is_pending(uint32_t irq);

/**
 * Get a bitmask of the IRQs that are both enabled and pending.
 */
uint32_t irq_poll(void);

#endif /* DRIVERS_IRQ_H */
