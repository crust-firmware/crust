/*
 * Copyright © 2017-2018 The Crust Firmware Authors.
 * SPDX-License-Identifier: (BSD-3-Clause OR GPL-2.0)
 */

#ifndef LIB_ERROR_H
#define LIB_ERROR_H

#define SUCCESS 0     /**< The operation succeeded. */
#define EAGAIN  (-1)  /**< Try again immediately. */
#define EBUSY   (-2)  /**< Try again later. */
#define EEXIST  (-3)  /**< The object already exists. */
#define EINVAL  (-4)  /**< The argument to the function was invalid. */
#define EIO     (-5)  /**< Communication with the hardware failed. */
#define ENOBUS  (-6)  /**< The bus for this device does not exist. */
#define ENODEV  (-7)  /**< The device does not exist. */
#define ENOTSUP (-8)  /**< The operation is not supported. */
#define EPERM   (-9)  /**< The operation is not permitted. */
#define ERANGE  (-10) /**< The argument to the function was out of range. */

#endif /* LIB_ERROR_H */
