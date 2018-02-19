/*
 * Copyright © 2017-2018 The Crust Firmware Authors.
 * SPDX-License-Identifier: (BSD-3-Clause OR GPL-2.0)
 */

#include <debug.h>
#include <stdbool.h>
#include <stddef.h>
#include <work.h>

/* Work item queue capacity. */
#define MAX_WORK_ITEMS 8

static size_t queue_head = 0;
static size_t queue_tail = MAX_WORK_ITEMS;

/* Queue of work items. */
static struct work_item work_items[MAX_WORK_ITEMS];

/**
 * Determine if the queue is empty using the given arguments. The values of
 * the head and tail range from 0 to 2 * MAX_WORK_ITEMS. This allows us to
 * distinguish between a full queue or an empty queue if the head is equal to
 * the tail or if the head and tail are exactly MAX_WORK_ITEMS elements apart,
 * respectively.
 *
 * @param  head The index of the head of the work_items queue.
 * @param  tail The index of the tail of the work_items queue.
 * @return      True if the queue is empty, otherwise false.
 */
static inline bool
queue_empty(size_t head, size_t tail)
{
	return (head + MAX_WORK_ITEMS) % (2 * MAX_WORK_ITEMS) == tail;
}

/**
 * Determine if the queue is full using the given arguments.
 *
 * @param  head The index of the head of the work_items queue.
 * @param  tail The index of the tail of the work_items queue.
 * @return      True if the queue is full, otherwise false.
 */
static inline bool
queue_full(size_t head, size_t tail)
{
	return head == tail;
}

/**
 * Find the next value of the given queue head or tail.
 *
 * @param  head The index of the head or tail of the work_items queue.
 * @return      The next value of the queue head or tail.
 */
static inline size_t
queue_next(size_t i)
{
	return (i + 1) % (2 * MAX_WORK_ITEMS);
}

void
process_work(void)
{
	/* Walk the queue. */
	while (!queue_empty(queue_head, queue_tail)) {
		size_t i = queue_head % MAX_WORK_ITEMS;
		work_items[i].fn(work_items[i].param);
		queue_head = queue_next(queue_head);
	}
}

void
queue_work(work_function fn, void *param)
{
	size_t i;

	/* Walk the queue and check for duplicates. */
	for (i = queue_head; !queue_empty(i, queue_tail); i = queue_next(i)) {
		if (work_items[i % MAX_WORK_ITEMS].fn == fn &&
		    work_items[i % MAX_WORK_ITEMS].param == param) {
			debug("(%p, %p) is already in the work queue",
			      (void *)(uintptr_t)fn, param);
			return;
		}
	}

	if (queue_full(queue_head, queue_tail))
		panic("Work queue is full");

	/* Add the work item to the tail of the queue. */
	work_items[queue_tail % MAX_WORK_ITEMS].fn    = fn;
	work_items[queue_tail % MAX_WORK_ITEMS].param = param;
	queue_tail = queue_next(queue_tail);
}
