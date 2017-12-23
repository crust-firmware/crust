/*
 * Copyright © 2017 Samuel Holland <samuel@sholland.org>
 * SPDX-License-Identifier: (BSD-3-Clause OR GPL-2.0)
 */

#ifndef DEBUG_H
#define DEBUG_H

#include <compiler.h>

#define LOG_STRING_PANIC   "\x01"
#define LOG_STRING_ERROR   "\x02"
#define LOG_STRING_WARNING "\x03"
#define LOG_STRING_INFO    "\x04"
#define LOG_STRING_DEBUG   "\x05"

enum {
	LOG_LEVEL_PANIC = 0,
	LOG_LEVEL_ERROR,
	LOG_LEVEL_WARNING,
	LOG_LEVEL_INFO,
#if DEBUG
	LOG_LEVEL_DEBUG,
#endif
	LOG_LEVELS
};

void log(const char *fmt, ...) __printf(1, 2);
noreturn void panic(const char *fmt, ...) __printf(1, 2);

#define panic(...) panic(LOG_STRING_PANIC __VA_ARGS__)
#define error(...) log(LOG_STRING_ERROR __VA_ARGS__)
#define warn(...)  log(LOG_STRING_WARNING __VA_ARGS__)
#define info(...)  log(LOG_STRING_INFO __VA_ARGS__)
#if DEBUG
#define debug(...) log(LOG_STRING_DEBUG __VA_ARGS__)
#else
#define debug(...) ((void)0)
#endif

#endif /* DEBUG_H */
