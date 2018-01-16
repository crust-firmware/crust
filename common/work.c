/*
 * Copyright © 2017 Drew Walters <drewwalters96@gmail.com>
 * SPDX-License-Identifier: (BSD-3-Clause OR GPL-2.0)
 */

#include <debug.h>
#include <stdbool.h>
#include <stddef.h>
#include <work.h>

/* Work item queue capacity. */
#define MAX_WORK_ITEMS 1

/* Queue of work items. */
static struct {
	work_function fn;
	void         *param;
} work_items[MAX_WORK_ITEMS];

void
process_work(void)
{
	bool found_work;

	do {
		found_work = false;
		/* Execute work items in the queue. */
		for (size_t i = 0; i < MAX_WORK_ITEMS; ++i) {
			if (work_items[i].fn != NULL) {
				found_work = true;
				work_items[i].fn(work_items[i].param);
				work_items[i].fn = NULL;
			}
		}
	} while (found_work);
}

void
queue_work(work_function fn, void *param)
{
	bool queued = false;

	/* Find first available index and verify work item is not queued. */
	for (size_t i = 0; i < MAX_WORK_ITEMS; ++i) {
		if (!queued && work_items[i].fn == NULL) {
			queued              = true;
			work_items[i].fn    = fn;
			work_items[i].param = param;
		} else if (work_items[i].fn == fn &&
		           work_items[i].param == param) {
			if (queued) {
				/* Remove duplicate work item from queue. */
				work_items[i].fn = NULL;
			}
			return;
		}
	}

	if (!queued)
		panic("Work queue is full");
}
