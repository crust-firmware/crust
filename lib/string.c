/*
 * Copyright © 2017 Samuel Holland <samuel@sholland.org>
 * SPDX-License-Identifier: (BSD-3-Clause OR GPL-2.0)
 */

#include <stddef.h>
#include <string.h>

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
