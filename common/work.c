/*
 * Copyright © 2017-2018 The Crust Firmware Authors.
 * SPDX-License-Identifier: BSD-3-Clause OR GPL-2.0-only
 */

#include <debug.h>
#include <interrupts.h>
#include <stdbool.h>
#include <stddef.h>
#include <work.h>

/* Work item queue capacity. */
#define MAX_WORK_ITEMS 8

static size_t queue_head = 0;
static size_t queue_tail = MAX_WORK_ITEMS;

/* Queue of work items. */
static struct handler work_items[MAX_WORK_ITEMS];

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
	callback_t *fn;
	uint32_t    flags;
	void *param;

	/* Walk the queue with interrupts disabled. */
	flags = disable_interrupts();
	while (!queue_empty(queue_head, queue_tail)) {
		size_t i = queue_head % MAX_WORK_ITEMS;
		/* First make a copy of the work function and its parameter. */
		fn    = work_items[i].fn;
		param = work_items[i].param;
		/* Then remove the it from the locked queue. */
		queue_head = queue_next(queue_head);
		restore_interrupts(flags);
		/* Call the work function with interrupts enabled. This is only
		 * safe if process_work() is never run concurrently, or else
		 * the other call could update queue_head behind our back. */
		fn(param);
		flags = disable_interrupts();
	}
	restore_interrupts(flags);
}

void
queue_work(callback_t *fn, void *param)
{
	size_t   i;
	uint32_t flags;

	/* Walk the queue with interrupts disabled and check for duplicates. */
	flags = disable_interrupts();
	for (i = queue_head; !queue_empty(i, queue_tail); i = queue_next(i)) {
		if (work_items[i % MAX_WORK_ITEMS].fn == fn &&
		    work_items[i % MAX_WORK_ITEMS].param == param) {
			restore_interrupts(flags);
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
	restore_interrupts(flags);
}
