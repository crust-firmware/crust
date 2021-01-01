/*
 * Copyright © 2021 The Crust Firmware Authors.
 * SPDX-License-Identifier: BSD-3-Clause OR GPL-2.0-only
 */

#ifndef IRQ_PRIVATE_H
#define IRQ_PRIVATE_H

#include <stdint.h>

/**
 * Poll for interrupts from the main PIO controller's EINT pins.
 */
uint32_t irq_poll_eint(void);

#endif /* IRQ_PRIVATE_H */
