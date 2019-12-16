/*
 * Copyright © 2017-2019 The Crust Firmware Authors.
 * SPDX-License-Identifier: BSD-3-Clause OR GPL-2.0-only
 */

#ifndef LIB_ERROR_H
#define LIB_ERROR_H

#define SUCCESS 0    /**< The operation succeeded. */
#define EBUSY   (-1) /**< Try again later. */
#define EEXIST  (-2) /**< The object already exists. */
#define EINVAL  (-3) /**< The argument to the function was invalid. */
#define EIO     (-4) /**< Communication with the hardware failed. */
#define ENODEV  (-5) /**< The device does not exist. */
#define ENOENT  (-6) /**< The entry does not exist. */
#define ENOTSUP (-7) /**< The operation is not supported. */
#define EPERM   (-8) /**< The operation is not permitted. */
#define ERANGE  (-9) /**< The argument to the function was out of range. */

#endif /* LIB_ERROR_H */
