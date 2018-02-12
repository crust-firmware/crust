/*
 * Copyright © 2017-2018 The Crust Firmware Authors.
 * SPDX-License-Identifier: (BSD-3-Clause OR GPL-2.0)
 */

#ifndef DRIVERS_PIO_SUNXI_PIO_H
#define DRIVERS_PIO_SUNXI_PIO_H

#define SUNXI_PIO_PIN(port, index) ((port) << 5 | ((index) & BITMASK(0, 5)))

extern const struct driver sunxi_pio_driver;

#endif /* DRIVERS_PIO_SUNXI_PIO_H */
