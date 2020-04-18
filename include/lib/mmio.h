/*
 * Copyright © 2017-2020 The Crust Firmware Authors.
 * SPDX-License-Identifier: BSD-3-Clause OR GPL-2.0-only
 */

#ifndef LIB_MMIO_H
#define LIB_MMIO_H

#include <stdint.h>

/**
 * Clear bits in a 32-bit MMIO register.
 *
 * @param addr The address of the register.
 * @param clr  The bits to clear.
 */
static inline void
mmio_clr_32(uintptr_t addr, uint32_t clr)
{
	volatile uint32_t *ptr = (void *)addr;

	*ptr &= ~clr;
}

/**
 * Clear and set bits in a 32-bit MMIO register.
 *
 * @param addr The address of the register.
 * @param clr  The bits to clear.
 * @param set  The bits to set.
 */
static inline void
mmio_clrset_32(uintptr_t addr, uint32_t clr, uint32_t set)
{
	volatile uint32_t *ptr = (void *)addr;

	*ptr = (*ptr & ~clr) | set;
}

/**
 * Get bits from a 32-bit MMIO register.
 *
 * @param addr The address of the register.
 * @param get  The bits to get.
 */
static inline uint32_t
mmio_get_32(uintptr_t addr, uint32_t get)
{
	volatile uint32_t *ptr = (void *)addr;

	return *ptr & get;
}

/**
 * Spin until all bits in a mask are set in a 32-bit MMIO register.
 *
 * @param addr The address of the register.
 * @param mask The bits that must all be set.
 */
static inline void
mmio_poll_32(uintptr_t addr, uint32_t mask)
{
	volatile uint32_t *ptr = (void *)addr;

	while ((*ptr & mask) != mask) {
		/* Do nothing. */
	}
}

/**
 * Spin until a value is present in a 32-bit MMIO register.
 *
 * @param addr The address of the register.
 * @param mask The bits to check.
 * @param val  The expected value for those bits.
 */
static inline void
mmio_polleq_32(uintptr_t addr, uint32_t mask, uint32_t val)
{
	volatile uint32_t *ptr = (void *)addr;

	while ((*ptr & mask) != val) {
		/* Do nothing. */
	}
}

/**
 * Spin until all bits in a mask are cleared in a 32-bit MMIO register.
 *
 * @param addr The address of the register.
 * @param mask The bits that must all be cleared.
 */
static inline void
mmio_pollz_32(uintptr_t addr, uint32_t mask)
{
	volatile uint32_t *ptr = (void *)addr;

	while ((*ptr & mask) != 0) {
		/* Do nothing. */
	}
}

/**
 * Read a 32-bit MMIO register.
 *
 * @param addr The address of the register.
 * @return     The value of the register.
 */
static inline uint32_t
mmio_read_32(uintptr_t addr)
{
	volatile uint32_t *ptr = (void *)addr;

	return *ptr;
}

/**
 * Set bits in a 32-bit MMIO register.
 *
 * @param addr The address of the register.
 * @param set  The bits to set.
 */
static inline void
mmio_set_32(uintptr_t addr, uint32_t set)
{
	volatile uint32_t *ptr = (void *)addr;

	*ptr |= set;
}

/**
 * Write a 32-bit MMIO register.
 *
 * @param addr The address of the register.
 * @param val  The new value of the register.
 */
static inline void
mmio_write_32(uintptr_t addr, uint32_t val)
{
	volatile uint32_t *ptr = (void *)addr;

	*ptr = val;
}

#endif /* LIB_MMIO_H */
