/*
 * Copyright © 2017-2022 The Crust Firmware Authors.
 * SPDX-License-Identifier: BSD-3-Clause OR GPL-2.0-only
 */

#ifndef COMMON_DEBUG_H
#define COMMON_DEBUG_H

#include <stdint.h>
#include <trap.h>

#define LOG_STRING_ERROR   "\x01"
#define LOG_STRING_WARNING "\x02"
#define LOG_STRING_INFO    "\x03"
#define LOG_STRING_DEBUG   "\x04"

#if CONFIG(ASSERT)
#if CONFIG(ASSERT_VERBOSE)
#define assert(e) ((void)((e) || (error("Assertion failed: %s (%s:%d)", #e, \
	                                __FILE__, __LINE__), trap(), 0)))
#else
#define assert(e) ((void)((e) || (trap(), 0)))
#endif
#else
#define assert(e) ((void)0)
#endif

enum {
	LOG_LEVEL_ERROR,
	LOG_LEVEL_WARNING,
	LOG_LEVEL_INFO,
	LOG_LEVEL_DEBUG,
	LOG_LEVELS
};

void hexdump(uintptr_t addr, uint32_t bytes);
void log(const char *fmt, ...) ATTRIBUTE(format(printf, 1, 2));

#define panic(...) (error(__VA_ARGS__), trap())
#define error(...) log(LOG_STRING_ERROR __VA_ARGS__)
#define warn(...)  log(LOG_STRING_WARNING __VA_ARGS__)
#define info(...)  log(LOG_STRING_INFO __VA_ARGS__)
#if CONFIG(DEBUG_LOG)
#define debug(...) log(LOG_STRING_DEBUG __VA_ARGS__)
#else
#define debug(...) ((void)0)
#endif

#if CONFIG(DEBUG_MONITOR)

void debug_monitor(void);

#else

static inline void
debug_monitor(void)
{
}

#endif

#if CONFIG(DEBUG_PRINT_BATTERY)

void debug_print_battery(void);

#else

static inline void
debug_print_battery(void)
{
}

#endif

#if CONFIG(DEBUG_PRINT_LATENCY)

void debug_print_latency(uint8_t current_state);

#else

static inline void
debug_print_latency(uint8_t current_state UNUSED)
{
}

#endif

#if CONFIG(DEBUG_PRINT_SPRS)

void debug_print_sprs(void);

#else

static inline void
debug_print_sprs(void)
{
}

#endif

#if CONFIG(DEBUG_VERIFY_DRAM)

void dram_save_checksum(void);
void dram_verify_checksum(void);

#else

static inline void
dram_save_checksum(void)
{
}

static inline void
dram_verify_checksum(void)
{
}

#endif

#endif /* COMMON_DEBUG_H */
