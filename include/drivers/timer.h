/*
 * Copyright © 2017-2018 The Crust Firmware Authors.
 * SPDX-License-Identifier: BSD-3-Clause OR GPL-2.0-only
 */

#ifndef DRIVERS_TIMER_H
#define DRIVERS_TIMER_H

#include <dm.h>
#include <intrusive.h>
#include <stdint.h>
#include <work.h>

#define TIMER_OPS(dev) \
	(&container_of((dev)->drv, struct timer_driver, drv)->ops)

struct timer_driver_ops {
	int (*get_timeout)(struct device *dev, uint32_t *timeout);
	int (*set_timeout)(struct device *dev, uint32_t timeout);
};

struct timer_driver {
	const struct driver           drv;
	const struct timer_driver_ops ops;
};

/**
 * Dequeue a function so it is not run on future timer ticks.
 */
int timer_cancel_periodic(callback_t *fn, void *param);

/**
 * Get the number of reference clock cycles until the next timer tick.
 */
int timer_get_timeout(uint32_t *timeout);

/**
 * Recalculate the time until the next scheduled timer tick.
 */
int timer_refresh(void);

/**
 * Register an available timer device with the timer framework.
 */
int timer_register_device(struct device *dev);

/**
 * Enqueue a function to run once at some point in the future.
 *
 * @param delay Offset from current time in reference clock cycles.
 */
int timer_run_delayed(callback_t *fn, void *param, uint32_t delay);

/**
 * Enqueue a function to run once on every periodic timer tick.
 */
int timer_run_periodic(callback_t *fn, void *param);

/**
 * Dispatch functions queued to run on this timer tick.
 *
 * This function is called from the interrupt handlers of timer devices.
 */
void timer_tick(void);

#endif /* DRIVERS_TIMER_H */
