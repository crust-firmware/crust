/*
 * Copyright © 2017-2022 The Crust Firmware Authors.
 * SPDX-License-Identifier: BSD-3-Clause OR GPL-2.0-only
 */

#ifndef LIB_COMPILER_H
#define LIB_COMPILER_H

/* Attributes */
#define ATTRIBUTE(...) __attribute__((__VA_ARGS__))
#define UNUSED         __attribute__((unused))
#define WEAK           __attribute__((weak))

/* Barriers */
#define barrier()      asm volatile ("" : : : "memory")

/* Builtins */
#define likely(e)      __builtin_expect(!!(e), 1)
#define unlikely(e)    __builtin_expect(e, 0)
#define unreachable()  __builtin_unreachable()

/* Keywords */
#define alignas        _Alignas
#define alignof        _Alignof
#define asm            __asm__
#define fallthrough    __attribute__((__fallthrough__))
#define noreturn       _Noreturn
#define static_assert  _Static_assert

/**
 * Calculate the size of a struct containing a flexible array member.
 *
 * @param type     The name of the structure type.
 * @param member   The name of the flexible array member.
 * @param elements The number of elements in the flexible array member.
 * @return         The total size of the type.
 */
#define sizeof_struct(type, member, elements) \
	(sizeof(type) + sizeof(*((type *)0)->member) * elements)

#endif /* LIB_COMPILER_H */
