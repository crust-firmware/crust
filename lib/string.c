/*
 * Copyright © 2017-2020 The Crust Firmware Authors.
 * SPDX-License-Identifier: BSD-3-Clause OR GPL-2.0-only
 */

#include <stddef.h>
#include <stdint.h>
#include <string.h>

#undef memcpy
#undef strlen

void *
memcpy(void *restrict dest, const void *restrict src, size_t n)
{
	char *d       = dest;
	const char *s = src;

	while (n-- > 0)
		*d++ = *s++;

	return dest;
}

size_t
strlen(const char *s)
{
	size_t len = 0;

	while (*s++)
		++len;

	return len;
}
