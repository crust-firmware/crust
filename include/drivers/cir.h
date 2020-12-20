/*
 * Copyright © 2020 The Crust Firmware Authors.
 * SPDX-License-Identifier: BSD-3-Clause OR GPL-2.0-only
 */

#ifndef DRIVERS_CIR_H
#define DRIVERS_CIR_H

#include <device.h>
#include <stddef.h>
#include <stdint.h>

#if CONFIG(CIR)

/**
 * Acquire a reference to the CIR receiver device suitable for polling.
 *
 * The reference must be released before resuming the rich OS.
 *
 * @return    A reference to the CIR receiver device.
 */
const struct device *cir_get(void);

/**
 * Check the CIR receiver for a wakeup condition.
 *
 * @param dev A reference to the CIR receiver device.
 * @return    Nonzero if the system should wake up, else zero.
 */
uint32_t cir_poll(const struct device *dev);

#else

static inline const struct device *
cir_get(void)
{
	return NULL;
}

static inline uint32_t
cir_poll(const struct device *dev UNUSED)
{
	return 0;
}

#endif

#endif /* DRIVERS_CIR_H */
