/*
 * Copyright © 2017-2020 The Crust Firmware Authors.
 * SPDX-License-Identifier: BSD-3-Clause OR GPL-2.0-only
 */

#ifndef DRIVERS_SERIAL_H
#define DRIVERS_SERIAL_H

/**
 * Read a character from the UART.
 *
 * @return The character read, or 0 if no character is available.
 */
char serial_getc(void);
void serial_putc(char c);
void serial_puts(const char *s);

#endif /* DRIVERS_SERIAL_H */
