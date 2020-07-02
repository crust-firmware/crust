/*
 * Copyright © 2017-2020 The Crust Firmware Authors.
 * SPDX-License-Identifier: BSD-3-Clause OR GPL-2.0-only
 */

#ifndef CLOCK_PRIVATE_H
#define CLOCK_PRIVATE_H

#include <clock.h>
#include <device.h>
#include <stdint.h>

#define CLOCK_DEVICE_STATE_INIT(n) \
	(struct device_state *) \
	&(char[sizeof_struct(struct clock_device_state, cs, n)]) { 0 }

struct clock_state {
	uint8_t refcount;
};

struct clock_device_state {
	struct device_state ds;
	struct clock_state  cs[];
};

struct clock_driver_ops {
	const struct clock_handle *
	         (*get_parent)(const struct clock_handle *clock);
	uint32_t (*get_rate)(const struct clock_handle *clock, uint32_t rate);
	uint32_t (*get_state)(const struct clock_handle *clock);
	void     (*set_state)(const struct clock_handle *clock,
	                      uint32_t state);
};

struct clock_driver {
	struct driver           drv;
	struct clock_driver_ops ops;
};

#endif /* CLOCK_PRIVATE_H */
