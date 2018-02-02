/*
 * Copyright © 2017-2018 The Crust Firmware Authors.
 * SPDX-License-Identifier: (BSD-3-Clause OR GPL-2.0)
 */

#ifndef WORK_H
#define WORK_H

/**
 * A function that performs asynchronous/delayed work.
 */
typedef void (*work_function)(void *);

/**
 * Structure representing a unit of work that can be done.
 */
struct work_item {
	work_function fn;
	void         *param;
};

/**
 * Execute each work item in the queue until it is empty.
 */
void process_work(void);

/**
 * Queue a work item, e.g. from an interrupt handler.
 */
void queue_work(work_function fn, void *param);

#endif /* WORK_H */
