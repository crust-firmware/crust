/*
 * Copyright © 2017-2018 The Crust Firmware Authors.
 * SPDX-License-Identifier: BSD-3-Clause OR GPL-2.0-only
 */

#ifndef COMMON_WORK_H
#define COMMON_WORK_H

/**
 * A function that performs asynchronous/delayed work.
 */
typedef void callback_t (void *);

/**
 * Structure representing a unit of work that can be done.
 */
struct handler {
	callback_t *fn;
	void       *param;
};

/**
 * Execute each work item in the queue until it is empty.
 */
void process_work(void);

/**
 * Queue a work item, e.g. from an interrupt handler.
 */
void queue_work(callback_t *fn, void *param);

#endif /* COMMON_WORK_H */
