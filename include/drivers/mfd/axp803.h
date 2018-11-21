/*
 * Copyright © 2017-2018 The Crust Firmware Authors.
 * SPDX-License-Identifier: (BSD-3-Clause OR GPL-2.0)
 */

#ifndef DRIVERS_MFD_AXP803_H
#define DRIVERS_MFD_AXP803_H

#include <dm.h>
#include <stdint.h>

#define AXP803_RSB_HWADDR 0x03a3
#define AXP803_RSB_RTADDR 0x2d

/**
 * Initialize an AXP803 PMIC.
 *
 * This function may fail with:
 *  EIO    There was a problem communicating with the hardware.
 *  ENODEV No AXP803 PMIC was found.
 *
 * @param dev  The device to check.
 * @return     Zero if an AXP803 is present and successfully initialized; any
 *             other value if it is not.
 */
int axp803_init_once(struct device *dev);

/**
 * Set one or more bits in a register in the AXP803 PMIC.
 *
 * This function may fail with:
 *  EIO    There was a problem communicating with the hardware.
 *
 * @param dev  A device associated with an AXP803 PMIC.
 * @param reg  The register within the PMIC to write.
 * @param bits A mask of bits to set in the register. Bits that are ones here
 *             will be set in the register; zeroes here will be left unchanged.
 * @return     Zero on success; a defined error code on failure.
 */
int axp803_reg_setbits(struct device *dev, uint8_t reg, uint8_t bits);

#endif /* DRIVERS_MFD_AXP803_H */
