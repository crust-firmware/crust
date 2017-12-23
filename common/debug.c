/*
 * Copyright © 2017 Samuel Holland <samuel@sholland.org>
 * SPDX-License-Identifier: (BSD-3-Clause OR GPL-2.0)
 */

#include <console.h>
#include <ctype.h>
#include <debug.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>

static char *prefixes[] = {
	"PANIC:\t ",
	"ERROR:\t ",
	"WARNING: ",
	"INFO:\t ",
#if DEBUG
	"DEBUG:\t ",
#endif
};

static void print_decimal(char sign, int width, bool zero, uint32_t num);
static void print_hex(int width, bool zero, uint32_t num);
static void print_padding(int width, bool zero);
static void print_signed(char sign, int width, bool zero, int32_t num);
static void print_string(const char *s);

void
log(const char *fmt, ...)
{
	bool zero;
	char c, sign;
	int  width;
	uintptr_t arg, level;
	va_list   args;

	assert(fmt);

	level = *fmt - 1;
	if (level < LOG_LEVELS) {
		print_string(prefixes[level]);
		++fmt;
	}
	va_start(args, fmt);
	while ((c = *fmt++)) {
		if (c != '%') {
			console_putc(c);
			continue;
		}
		arg   = va_arg(args, uintptr_t);
		sign  = '\0';
		width = 0;
		zero  = false;
conversion:
		switch ((c = *fmt++)) {
		case ' ':
			if (!sign)
				sign = ' ';
			goto conversion;
		case '%':
			console_putc(c);
			break;
		case '+':
			sign = '+';
			goto conversion;
		case 'c':
			print_padding(width - 1, zero);
			console_putc(arg);
			break;
		case 'd':
		case 'i':
			print_signed(sign, width, zero, arg);
			break;
		case 'p':
			/* "%p" behaves like "0x%08x". */
			print_string("0x");
			width = 2 * sizeof(arg);
			zero  = true;
		case 'x':
			print_hex(width, zero, arg);
			break;
		case 's':
			assert(arg);
			print_padding(width - strlen((const char *)arg), zero);
			print_string((const char *)arg);
			break;
		case 'u':
			print_decimal(sign, width, zero, arg);
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
		console_putc('\n');
}

static void
print_decimal(char sign, int width, bool zero, uint32_t num)
{
	unsigned digits  = 1;
	unsigned divisor = 1;

	while (divisor <= num / 10) {
		++digits;
		divisor *= 10;
	}
	if (sign) {
		console_putc(sign);
		--width;
	}
	print_padding(width - digits, zero);
	while (digits--) {
		uint32_t digit = 0;
		while (num >= divisor) {
			num -= divisor;
			++digit;
		}
		console_putc(digit + '0');
		divisor /= 10;
	}
}

static void
print_hex(int width, bool zero, uint32_t num)
{
	unsigned bits   = 8 * sizeof(num);
	unsigned digits = 2 * sizeof(num);

	while (digits > 1) {
		if ((num >> (bits - 4)) & 0xf)
			break;
		--digits;
		num <<= 4;
	}
	print_padding(width - digits, zero);
	while (digits--) {
		uint32_t digit = (num >> (bits - 4)) & 0xf;
		console_putc(digit < 10 ? digit + '0' : digit - 10 + 'a');
		num <<= 4;
	}
}

static void
print_padding(int width, bool zero)
{
	for (int i = 0; i < width; ++i)
		console_putc(zero ? '0' : ' ');
}

static void
print_signed(char sign, int width, bool zero, int32_t num)
{
	if (num < 0)
		print_decimal('-', width, zero, -num);
	else
		print_decimal(sign, width, zero, num);
}

static void
print_string(const char *s)
{
	char c;

	while ((c = *s++))
		console_putc(c);
}
