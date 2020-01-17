/*
 * Copyright © 2005-2014 Rich Felker, et al.
 * Copyright © 2017-2020 The Crust Firmware Authors.
 * SPDX-License-Identifier: MIT
 */

#ifndef STDLIB_CTYPE_H
#define STDLIB_CTYPE_H

#include <stdbool.h>

#define isalpha(a) (((unsigned)(a) | 32) - 'a' < 26)
#define isascii(a) ((unsigned)(a) < 128)
#define isdigit(a) ((unsigned)(a) - '0' < 10)
#define isgraph(a) ((unsigned)(a) - 0x21 < 0x5e)
#define islower(a) ((unsigned)(a) - 'a' < 26)
#define isprint(a) ((unsigned)(a) - 0x20 < 0x5f)
#define isupper(a) ((unsigned)(a) - 'A' < 26)
#define tolower(a) ((a) | 0x20)
#define toupper(a) ((a) & 0x5f)

static inline bool
isalnum(char c)
{
	return isalpha(c) || isdigit(c);
}

static inline bool
isblank(char c)
{
	return c == ' ' || c == '\t';
}

static inline bool
iscntrl(char c)
{
	return (unsigned)c < 0x20 || c == 0x7f;
}

static inline bool
ispunct(char c)
{
	return isgraph(c) && !isalnum(c);
}

static inline bool
isspace(char c)
{
	return c == ' ' || (unsigned)c - '\t' < 5;
}

static inline bool
isxdigit(char c)
{
	return isdigit(c) || ((unsigned)c | 32) - 'a' < 6;
}

#endif /* STDLIB_CTYPE_H */
