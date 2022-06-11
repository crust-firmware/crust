/*
 * Copyright © 2020-2022 The Crust Firmware Authors.
 * SPDX-License-Identifier: BSD-3-Clause OR GPL-2.0-only
 */

#ifndef DRIVERS_DRAM_H
#define DRIVERS_DRAM_H

/**
 * Initialize the DRAM controller driver.
 */
void dram_init(void);

/**
 * Resume the DRAM controller and exit self-refresh.
 */
void dram_resume(void);

/**
 * Enter self-refresh and suspend the DRAM controller.
 */
void dram_suspend(void);

#endif /* DRIVERS_DRAM_H */
