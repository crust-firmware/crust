/*
 * Copyright © 2017-2021 The Crust Firmware Authors.
 * SPDX-License-Identifier: BSD-3-Clause OR GPL-2.0-only
 */

#ifndef DRIVERS_SERIAL_H
#define DRIVERS_SERIAL_H

#include <stdbool.h>

#if CONFIG(SERIAL)

/**
 * Read a character from the UART.
 *
 * @return The character read, or 0 if no character is available.
 */
char serial_getc(void);
void serial_putc(char c);
void serial_puts(const char *s);

/**
 * Initialize the UART.
 */
void serial_init(void);

/**
 * Verify that the UART is ready to use.
 *
 * This function must be called before performing any I/O. Other serial I/O
 * functions may only be called if this function returns true.
 */
bool serial_ready(void);

#else

static inline char
serial_getc(void)
{
	return 0;
}

static inline void
serial_putc(char c UNUSED)
{
}

static inline void
serial_puts(const char *s UNUSED)
{
}

static inline void
serial_init(void)
{
}

static inline bool
serial_ready(void)
{
	return false;
}

#endif

#endif /* DRIVERS_SERIAL_H */
