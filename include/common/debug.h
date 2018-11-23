/*
 * Copyright © 2017-2018 The Crust Firmware Authors.
 * SPDX-License-Identifier: BSD-3-Clause OR GPL-2.0-only
 */

#ifndef COMMON_DEBUG_H
#define COMMON_DEBUG_H

#include <compiler.h>
#include <stdint.h>

#define LOG_STRING_PANIC   "\x01"
#define LOG_STRING_ERROR   "\x02"
#define LOG_STRING_WARNING "\x03"
#define LOG_STRING_INFO    "\x04"
#define LOG_STRING_DEBUG   "\x05"

#if CONFIG_ASSERT
#if CONFIG_ASSERT_VERBOSE
#define assert(e) ((void)((e) || (panic("Assertion failed: %s (%s:%d)", #e, \
	                                __FILE__, __LINE__), 0)))
#else
#define assert(e) ((void)((e) || (panic("Assertion failed: %d", __LINE__), 0)))
#endif
#else
#define assert(e) ((void)0)
#endif

enum {
	LOG_LEVEL_PANIC = 0,
	LOG_LEVEL_ERROR,
	LOG_LEVEL_WARNING,
	LOG_LEVEL_INFO,
	LOG_LEVEL_DEBUG,
	LOG_LEVELS
};

void hexdump(uintptr_t addr, uint32_t bytes);
void log(const char *fmt, ...) __printf(1, 2);
noreturn void panic(const char *fmt, ...) __printf(1, 2);

#define panic(...) panic(LOG_STRING_PANIC __VA_ARGS__)
#define error(...) log(LOG_STRING_ERROR __VA_ARGS__)
#define warn(...)  log(LOG_STRING_WARNING __VA_ARGS__)
#define info(...)  log(LOG_STRING_INFO __VA_ARGS__)
#if CONFIG_DEBUG_LOG
#define debug(...) log(LOG_STRING_DEBUG __VA_ARGS__)
#else
#define debug(...) ((void)0)
#endif

#endif /* COMMON_DEBUG_H */
