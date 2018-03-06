/*
 * Copyright © 2017-2018 The Crust Firmware Authors.
 * SPDX-License-Identifier: (BSD-3-Clause OR GPL-2.0)
 */

#ifndef STDLIB_STRING_H
#define STDLIB_STRING_H

#include <stddef.h>

void *memcpy(void *restrict dest, const void *restrict src, size_t n);
int strcmp(const char *a, const char *b);
size_t strlen(const char *s);

#endif /* STDLIB_STRING_H */
