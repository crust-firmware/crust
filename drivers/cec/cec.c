/*
 * Copyright © 2021-2022 The Crust Firmware Authors.
 * SPDX-License-Identifier: BSD-3-Clause OR GPL-2.0-only
 */

#include <cec.h>
#include <device.h>
#include <cec/dw-hdmi-cec.h>

const struct device *
cec_get(void)
{
	return device_get_or_null(&hdmi_cec.dev);
}

uint32_t
cec_poll(const struct device *dev)
{
	if (!dev)
		return 0;

	return dw_hdmi_cec_poll(dev);
}
