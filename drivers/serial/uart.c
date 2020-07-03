/*
 * Copyright © 2017-2020 The Crust Firmware Authors.
 * SPDX-License-Identifier: BSD-3-Clause OR GPL-2.0-only
 */

#include "uart.h"

const struct driver uart_driver = {
	.probe   = simple_device_probe,
	.release = simple_device_release,
};
