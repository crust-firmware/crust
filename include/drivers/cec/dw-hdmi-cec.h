/*
 * Copyright © 2021-2022 The Crust Firmware Authors.
 * SPDX-License-Identifier: BSD-3-Clause OR GPL-2.0-only
 */

#ifndef DRIVERS_CEC_DW_HDMI_CEC_H
#define DRIVERS_CEC_DW_HDMI_CEC_H

#include <clock.h>
#include <device.h>
#include <stdint.h>

struct dw_hdmi_cec {
	struct device       dev;
	struct clock_handle bus_clock;
	uintptr_t           regs;
};

extern const struct dw_hdmi_cec hdmi_cec;

uint32_t dw_hdmi_cec_poll(const struct device *dev);

#endif /* DRIVERS_CEC_DW_HDMI_CEC_H */
