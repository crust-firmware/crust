/*
 * Copyright © 2017-2018 The Crust Firmware Authors.
 * SPDX-License-Identifier: (BSD-3-Clause OR GPL-2.0)
 */

#ifndef ERROR_H
#define ERROR_H

#define SUCCESS 0    /* The operation succeeded. */
#define EAGAIN  (-1) /* Try again immediately. */
#define EBUSY   (-2) /* Try again later. */
#define EEXIST  (-3) /* The object already exists. */
#define EIO     (-4) /* Communication with the hardware failed. */
#define ENOBUS  (-5) /* The bus for this device does not exist. */
#define ENODEV  (-6) /* This device does not exist. */
#define ENOTSUP (-7) /* The operation is not supported. */

#endif /* ERROR_H */
