/*
 * Copyright © 2017-2021 The Crust Firmware Authors.
 * SPDX-License-Identifier: BSD-3-Clause OR GPL-2.0-only
 */

#ifndef DRIVERS_IRQ_H
#define DRIVERS_IRQ_H

#include <stdint.h>

/**
 * Determine if any enabled IRQ requires AVCC in order to be received.
 *
 * @return Nonzero if some IRQ requires AVCC, else zero.
 */
uint32_t irq_needs_avcc(void);

/**
 * Determine if any enabled IRQ requires VDD_SYS in order to be received.
 *
 * @return Nonzero if some IRQ requires VDD_SYS, else zero.
 */
uint32_t irq_needs_vdd_sys(void);

/**
 * Get a bitmask of the IRQs that are both enabled and pending.
 *
 * @return Nonzero if some IRQ is enabled and pending, else zero.
 */
uint32_t irq_poll(void);

#endif /* DRIVERS_IRQ_H */
