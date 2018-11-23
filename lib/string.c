/*
 * Copyright © 2017-2018 The Crust Firmware Authors.
 * SPDX-License-Identifier: BSD-3-Clause OR GPL-2.0-only
 */

#include <stddef.h>
#include <stdint.h>
#include <string.h>

#undef memcpy
#undef strcmp
#undef strlen
#undef strncpy

void *
memcpy(void *restrict dest, const void *restrict src, size_t n)
{
	char *d       = dest;
	const char *s = src;

	while (n-- > 0)
		*d++ = *s++;

	return dest;
}

int
strcmp(const char *a, const char *b)
{
	while (*a && *a == *b) {
		++a;
		++b;
	}

	return *a - *b;
}

size_t
strlen(const char *s)
{
	size_t len = 0;

	while (*s++)
		++len;

	return len;
}

char *
strncpy(char *restrict dest, const char *restrict src, size_t n)
{
	char *d       = dest;
	const char *s = src;

	while (n-- > 0) {
		*d++ = *s;
		s   += *s != 0;
	}

	return dest;
}

char *
strncpy_swap(char *restrict dest, const char *restrict src, size_t n)
{
	uintptr_t   d = (uintptr_t)dest;
	const char *s = src;

	while (n-- > 0) {
		*(char *)(d++ ^ 3) = *s;
		s += *s != 0;
	}

	return dest;
}
