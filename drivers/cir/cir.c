/*
 * Copyright © 2020-2022 The Crust Firmware Authors.
 * SPDX-License-Identifier: BSD-3-Clause OR GPL-2.0-only
 */

#include <cir.h>
#include <device.h>
#include <cir/sunxi-cir.h>

const struct device *
cir_get(void)
{
	return device_get_or_null(&r_cir_rx.dev);
}

uint32_t
cir_poll(const struct device *dev)
{
	return sunxi_cir_poll(dev) == CONFIG_CIR_WAKE_CODE;
}
