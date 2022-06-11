/*
 * Copyright © 2021-2022 The Crust Firmware Authors.
 * SPDX-License-Identifier: BSD-3-Clause OR GPL-2.0-only
 */

#ifndef DRIVERS_CEC_H
#define DRIVERS_CEC_H

#include <device.h>
#include <stddef.h>
#include <stdint.h>

#if CONFIG(CEC)

/**
 * Acquire a reference to the CEC receiver device suitable for polling.
 *
 * The reference must be released before resuming the rich OS.
 *
 * @return    A reference to the CEC receiver device.
 */
const struct device *cec_get(void);

/**
 * Check the CEC receiver for a wakeup condition.
 *
 * @param dev A reference to the CEC receiver device.
 * @return    Nonzero if the system should wake up, else zero.
 */
uint32_t cec_poll(const struct device *dev);

#else

static inline const struct device *
cec_get(void)
{
	return NULL;
}

static inline uint32_t
cec_poll(const struct device *dev UNUSED)
{
	return 0;
}

#endif

#endif /* DRIVERS_CEC_H */
