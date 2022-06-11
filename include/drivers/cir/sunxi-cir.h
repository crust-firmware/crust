/*
 * Copyright © 2020-2022 The Crust Firmware Authors.
 * SPDX-License-Identifier: BSD-3-Clause OR GPL-2.0-only
 */

#ifndef DRIVERS_CIR_SUNXI_CIR_H
#define DRIVERS_CIR_SUNXI_CIR_H

#include <clock.h>
#include <device.h>
#include <gpio.h>
#include <stdint.h>

struct sunxi_cir {
	struct device       dev;
	struct clock_handle bus_clock;
	struct clock_handle mod_clock;
	struct gpio_handle  pin;
	uintptr_t           regs;
};

extern const struct sunxi_cir r_cir_rx;

uint32_t sunxi_cir_poll(const struct device *dev);

#endif /* DRIVERS_CIR_SUNXI_CIR_H */
