/*
 * Copyright © 2017-2020 The Crust Firmware Authors.
 * SPDX-License-Identifier: BSD-3-Clause OR GPL-2.0-only
 */

#include <ctype.h>
#include <debug.h>
#include <division.h>
#include <serial.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>

#define BYTES_PER_ROW  16
#define BYTES_PER_WORD sizeof(uint32_t)

static char *prefixes[LOG_LEVELS] = {
	"PANIC:\t ",
	"ERROR:\t ",
	"WARNING: ",
	"INFO:\t ",
	"DEBUG:\t ",
};

static void print_number(uint32_t num, int base, int width, bool zero);
static void print_signed(int32_t num, int base, int width, bool zero);

void
hexdump(uintptr_t addr, uint32_t bytes)
{
	uintptr_t start;

	/* Always start at a multiple of BYTES_PER_ROW. */
	addr &= ~(BYTES_PER_ROW - 1);
	for (start = addr; addr - start < bytes; addr += BYTES_PER_ROW) {
		uint32_t *words = (uint32_t *)addr;

		/* This assumes BYTES_PER_ROW is 16, which it will always be.
		 * It's more of an informational constant, not a variable. */
		log("%08x: %08x %08x %08x %08x  ",
		    addr, words[0], words[1], words[2], words[3]);

		/* The ARISC processor's data lines are swapped in hardware for
		 * compatibility with the little-endian ARM CPUs. To examine
		 * individual bytes, we must reverse each group of 4 bytes. */
		for (int i = BYTES_PER_WORD - 1; i < BYTES_PER_ROW; --i) {
			char c = ((char *)addr)[i];
			serial_putc(isprint(c) ? c : '.');
			if (i % BYTES_PER_WORD == 0)
				i += 2 * BYTES_PER_WORD;
		}
		serial_putc('\n');
	}
}

void
log(const char *fmt, ...)
{
	bool zero;
	char c;
	int  width;
	uintptr_t arg, level;
	va_list args;

	assert(fmt);

	level = *fmt - 1;
	if (level < LOG_LEVELS) {
		serial_puts(prefixes[level]);
		++fmt;
	}
	va_start(args, fmt);
	while ((c = *fmt++)) {
		if (c != '%') {
			serial_putc(c);
			continue;
		}
		if (*fmt == '%') {
			++fmt;
			serial_putc(c);
			continue;
		}
		arg   = va_arg(args, uintptr_t);
		width = 0;
		zero  = false;
conversion:
		switch ((c = *fmt++)) {
		case 'c':
			serial_putc(arg);
			break;
		case 'd':
		case 'i':
			print_signed(arg, 10, width, zero);
			break;
		case 'p':
			/* "%p" behaves like "0x%08x". */
			serial_puts("0x");
			print_number(arg, 16, 2 * sizeof(arg), true);
			break;
		case 'x':
			print_number(arg, 16, width, zero);
			break;
		case 's':
			assert(arg);
			serial_puts((const char *)arg);
			break;
		case 'u':
			print_number(arg, 10, width, zero);
			break;
		default:
			assert(c);
			if (c == '0' && width == 0)
				zero = true;
			else if (isdigit(c))
				width = 10 * width + (c - '0');
			goto conversion;
		}
	}
	va_end(args);
	if (level < LOG_LEVELS)
		serial_putc('\n');
}

static void
print_number(uint32_t num, int base, int width, bool zero)
{
	static const char chars[16] = "0123456789abcdef";
	char digits[3 * sizeof(num)];
	int  i = 0;

	do {
		digits[i++] = chars[udivmod(&num, base)];
	} while (num);
	while (width-- > i)
		serial_putc(zero ? '0' : ' ');
	while (i--)
		serial_putc(digits[i]);
}

static void
print_signed(int32_t num, int base, int width, bool zero)
{
	if (num < 0) {
		serial_putc('-');
		print_number(-num, base, width ? width - 1 : width, zero);
	} else {
		print_number(num, base, width, zero);
	}
}
