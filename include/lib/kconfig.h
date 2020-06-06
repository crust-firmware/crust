/*
 * Copyright © 2017-2020 The Crust Firmware Authors.
 * SPDX-License-Identifier: BSD-3-Clause OR GPL-2.0-only
 */

#ifndef LIB_KCONFIG_H
#define LIB_KCONFIG_H

/* These macros define the option values that will match (here just 1), as
 * well as the action that will be taken when a match is found. They work by
 * expanding a single argument into two arguments in the case of a match. */
#define __kcm_self_1 "this is junk", __kca_self
#define __kcm_init_1 "this is junk", __kca_init
#define __kcm_true_1 "this is junk", __kca_true

/* These macros define the result actions/expressions. Since __kc_indirect
 * consumes exactly two arguments, actions in the matching case have an extra
 * parameter (the one inserted by the match macros above). */
#define __kca_null(...)
#define __kca_self(junk, ...)      __VA_ARGS__
#define __kca_init(junk, ...)      __VA_ARGS__,
#define __kca_false(...)           0
#define __kca_true(junk, ...)      1

/* These macros split the comma-separated match expression (if present) into
 * two arguments, and then call the action named in the second argument. */
#define __kc_concat(a, b)          a ## b
#define __kc_do(junk, action, ...) action(__VA_ARGS__)
#define __kc_expand(...)           __kc_do(__VA_ARGS__)
#define __kc_tokenize(...)         __VA_ARGS__

/**
 * Expands to the remainder of the argument list if the option is enabled;
 * otherwise expands to the empty string.
 */
#define IF_ENABLED(option, ...) \
	__kc_expand(__kc_concat(__kcm_self_, option), __kca_null, __VA_ARGS__)

/**
 * Expands to the remainder of the argument list plus a comma if the option is
 * enabled; otherwise expands to the empty string. This is generally useful in
 * structure and array initializers.
 */
#define IF_ENABLED_INIT(option, ...) \
	__kc_expand(__kc_concat(__kcm_init_, option), __kca_null, __VA_ARGS__)

/**
 * Expands to the token 1 if the option is enabled; otherwise expands to 0.
 */
#define IS_ENABLED(option) \
	__kc_expand(__kc_concat(__kcm_true_, option), __kca_false, junk)

/**
 * Shorthand for IS_ENABLED(CONFIG_option).
 *
 * Expands to the token 1 if the option is enabled; otherwise expands to 0.
 */
#define CONFIG(option) \
	IS_ENABLED(__kc_tokenize(CONFIG_ ## option))

#endif /* LIB_KCONFIG_H */
