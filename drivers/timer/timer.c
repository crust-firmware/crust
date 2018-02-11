/*
 * Copyright © 2017-2018 The Crust Firmware Authors.
 * SPDX-License-Identifier: (BSD-3-Clause OR GPL-2.0)
 */

#include <compiler.h>
#include <debug.h>
#include <dm.h>
#include <error.h>
#include <stddef.h>
#include <drivers/timer.h>
#include <drivers/wallclock.h>

#define MAX_PERIODIC_ITEMS 1

/* One second at a reference clock rate of 24MHz. */
#define TICK_INTERVAL      24000000

static uint64_t last_tick;
static struct work_item periodic_work_items[MAX_PERIODIC_ITEMS];
static struct device   *timer;

int
timer_cancel_periodic(work_function fn, void *param)
{
	assert(fn);

	/* Walk the list and try to find a matching work item. */
	for (size_t i = 0; i < MAX_PERIODIC_ITEMS; ++i) {
		if (periodic_work_items[i].fn == fn &&
		    periodic_work_items[i].param == param) {
			periodic_work_items[i].fn = NULL;
			return SUCCESS;
		}
	}

	return EINVAL;
}

int
timer_device_register(struct device *dev)
{
	if (timer != NULL)
		return EEXIST;

	timer = dev;

	return SUCCESS;
}

int
timer_get_timeout(uint32_t *timeout)
{
	assert(timeout);

	if (timer == NULL)
		return ENODEV;

	return TIMER_OPS(timer)->get_timeout(timer, timeout);
}

int
timer_refresh(void)
{
	uint64_t current_time = wallclock_read();

	if (unlikely(timer == NULL))
		return ENODEV;

	if (unlikely(last_tick == 0))
		last_tick = current_time;
	while (last_tick <= current_time)
		last_tick += TICK_INTERVAL;

	return TIMER_OPS(timer)->set_timeout(timer, last_tick - current_time);
}

int
timer_run_delayed(work_function fn __unused, void *param __unused,
                  uint32_t delay __unused)
{
	return ENOTSUP;
}

int
timer_run_periodic(work_function fn, void *param)
{
	assert(fn);

	/* Find the first available index (does not handle duplicates). */
	for (size_t i = 0; i < MAX_PERIODIC_ITEMS; ++i) {
		if (periodic_work_items[i].fn == NULL) {
			periodic_work_items[i].fn    = fn;
			periodic_work_items[i].param = param;
			/* Start ticking. */
			return timer_refresh();
		}
	}

	panic("Periodic work queue is full");
}

void
timer_tick(void)
{
	/* Walk through periodic work items and add each to the work queue. */
	for (size_t i = 0; i < MAX_PERIODIC_ITEMS; ++i) {
		if (periodic_work_items[i].fn != NULL) {
			queue_work(periodic_work_items[i].fn,
			           periodic_work_items[i].param);
		}
	}

	timer_refresh();
}
