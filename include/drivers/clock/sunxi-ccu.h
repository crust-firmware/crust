/*
 * Copyright © 2017-2018 The Crust Firmware Authors.
 * SPDX-License-Identifier: (BSD-3-Clause OR GPL-2.0)
 */

#ifndef DRIVERS_CLOCK_SUNXI_CCU_H
#define DRIVERS_CLOCK_SUNXI_CCU_H

#define CCU_GATE(n)      (((n) & 0xffff) << 16)
#define CCU_GET_GATE(n)  (((n) >> 16) & 0xffff)
#define CCU_RESET(n)     ((n) & 0xffff)
#define CCU_GET_RESET(n) ((n) & 0xffff)

extern const struct driver sunxi_ccu_driver;

#endif /* DRIVERS_CLOCK_SUNXI_CCU_H */
