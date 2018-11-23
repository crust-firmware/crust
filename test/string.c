/*
 * Copyright © 2017-2018 The Crust Firmware Authors.
 * SPDX-License-Identifier: BSD-3-Clause OR GPL-2.0-only
 */

#include <stddef.h>
#include <string.h>
#include <unity.h>
#include <util.h>

static void
test_memcpy(void)
{
	/* Test a valid string.*/
	char src[30] = "This is a test of Memcpy";
	char dest[30];

	memcpy(dest, src, (sizeof(dest)));
	TEST_ASSERT_EQUAL_STRING(src, dest);

	/* Test a valid string with special characters. */
	char src2[50] = "This_is_a_test_of_Memcpy!";
	char dest2[50];
	memcpy(dest2, src2, sizeof(dest2));
	TEST_ASSERT_EQUAL_STRING(src2, dest2);

	/* Test with a uint8_t array. */
	uint8_t src3[10] = { 0x00, 0x01 };
	uint8_t dest3[10];
	memcpy(dest3, src3, sizeof(dest3));
	TEST_ASSERT_EQUAL_UINT8_ARRAY(src3, dest3, ARRAY_SIZE(dest3));

	/* Test with a uint32_t array. */
	uint32_t src4[10] = { 0x00, 0x01 };
	uint32_t dest4[10];
	memcpy(dest4, src4, sizeof(dest4));
	TEST_ASSERT_EQUAL_UINT32_ARRAY(src4, dest4, ARRAY_SIZE(dest4));
}

static void
test_strcmp(void)
{
	/* Test with long string. */
	TEST_ASSERT_EQUAL(0, strcmp("Here is a string", "Here is a string"));

	/* Test with medium string. */
	TEST_ASSERT_EQUAL(0, strcmp("Hello World", "Hello World"));

	/* Test with short string. */
	TEST_ASSERT_EQUAL(0, strcmp("A", "A"));

	/* Test with numbers in string. */
	TEST_ASSERT_EQUAL(0, strcmp("He11o W0rld", "He11o W0rld"));

	/* Test with special characters. */
	TEST_ASSERT_EQUAL(0, strcmp("Hello_World!", "Hello_World!"));

	/* Test string with only a space. */
	TEST_ASSERT_EQUAL(0, strcmp(" ", " "));

	/* Test an empty string. */
	TEST_ASSERT_EQUAL(0, strcmp("", ""));

	/* Test two different strings. */
	int result = strcmp("Here is a string", "Here is a different string");
	TEST_ASSERT(result > 0);

	int result2 = strcmp("Here is a different string", "Here is a string");
	TEST_ASSERT(result2 < 0);
}

static void
test_strlen(void)
{
	/* Test with long string. */
	TEST_ASSERT_EQUAL(59, strlen("Here is a long string. "
	                             "This goes above thirty-two charcters"));

	/* Test with medium string. */
	TEST_ASSERT_EQUAL(23, strlen("Here is a medium string"));

	/* Test with short string. */
	TEST_ASSERT_EQUAL(6, strlen("String"));

	/* Test with numbers in string. */
	TEST_ASSERT_EQUAL(7, strlen("String7"));

	/* Test with special characters. */
	TEST_ASSERT_EQUAL(8, strlen("Str-ing!"));

	/* Test with empty string. */
	TEST_ASSERT_EQUAL(0, strlen(""));
}

static void
test_strncpy(void)
{
	char dest10[10];
	char dest16[16];
	char dest20[20];
	const char *expected10 = "test sourc";
	const char *expected16 = "test source str";
	const char *expected20 = "test source str\0\0\0\0";
	const char *src        = "test source str";

	memset(dest10, 0xff, sizeof(dest10));
	strncpy(dest10, src, sizeof(dest10));
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected10, dest10, sizeof(dest10));

	memset(dest16, 0xff, sizeof(dest16));
	strncpy(dest16, src, sizeof(dest16));
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected16, dest16, sizeof(dest16));

	memset(dest20, 0xff, sizeof(dest20));
	strncpy(dest20, src, sizeof(dest20));
	TEST_ASSERT_EQUAL_UINT8_ARRAY(expected20, dest20, sizeof(dest20));

	char zero = '\0';
	char output[50];
	char zeroes[50];

	memset(output, 0xaa, sizeof(output));
	memset(zeroes, 0x00, sizeof(zeroes));
	strncpy(output, &zero, sizeof(output));
	TEST_ASSERT_EQUAL_UINT8_ARRAY(zeroes, output, sizeof(output));
}

int
main(void)
{
	UNITY_BEGIN();
	RUN_TEST(test_memcpy);
	RUN_TEST(test_strcmp);
	RUN_TEST(test_strlen);
	RUN_TEST(test_strncpy);
	return UNITY_END();
}
