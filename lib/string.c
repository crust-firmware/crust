/*
 * Copyright © 2017-2018 The Crust Firmware Authors.
 * SPDX-License-Identifier: (BSD-3-Clause OR GPL-2.0)
 */

#include <stddef.h>
#include <string.h>

void *
memcpy(void *dest, void *src, size_t n)
{
	char *d = dest;
	char *s = src;

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
