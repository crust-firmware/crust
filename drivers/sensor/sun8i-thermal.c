/*
 * Copyright © 2016, ARM Limited and Contributors. All rights reserved.
 * Copyright © 2017-2018 The Crust Firmware Authors.
 * SPDX-License-Identifier: (BSD-3-Clause OR GPL-2.0)
 */

#include <debug.h>
#include <error.h>
#include <limits.h>
#include <mmio.h>
#include <sensor.h>
#include <sensor/sun8i-thermal.h>

#define THS0_DATA_REG          0x80
#define THS1_DATA_REG          0x84
#define THS2_DATA_REG          0x88

#define THS_CTL_REG0           0x00
#define THS_CTL_REG1           0x04
#define THS_CTL_REG2           0x40

#define THS_FILTER_CONTROL_REG 0x70

struct sun8i_thermal_sensor_info {
	struct   sensor_info info;
	const uint16_t       address;
};

enum {
	THS0,
	THS1,
	THS2,
	SENSOR_COUNT
};

static struct sun8i_thermal_sensor_info thermal_sensors[SENSOR_COUNT] = {
	[THS0] = {
		.info = {
			.name       = "ths0",
			.offset     = 254300,
			.multiplier = -117,
		},
		.address = THS0_DATA_REG,
	},
	[THS1] = {
		.info = {
			.name       = "ths1",
			.offset     = 254300,
			.multiplier = -117,
		},
		.address = THS1_DATA_REG,
	},
	[THS2] = {
		.info = {
			.name       = "ths2",
			.offset     = 254300,
			.multiplier = -117,
		},
		.address = THS2_DATA_REG,
	},
};

static struct sensor_info *
sun8i_thermal_get_info(struct device *dev __unused, uint8_t id)
{
	assert(id < SENSOR_COUNT);

	return &thermal_sensors[id].info;
}

static int
sun8i_thermal_read_raw(struct device *dev, uint8_t id, uint32_t *raw)
{
	uint16_t addr = thermal_sensors[id].address;

	*raw = mmio_read32(dev->regs + addr) & BITMASK(0, 12);

	return SUCCESS;
}

static int
sun8i_thermal_probe(struct device *dev)
{
	uint32_t reg;
	int err;

	/* Set up two clocks for the thermal sensor. */
	if ((err = dm_setup_clocks(dev, 2)))
		return err;

	/* Start calibration. */
	mmio_write32(dev->regs + THS_CTL_REG1, BIT(17));
	while (mmio_read32(dev->regs + THS_CTL_REG1) & BIT(17)) {
		/* Wait for for calibration to clear. */
	}

	/* Set aquire times to 0.1ms. */
	mmio_write32(dev->regs + THS_CTL_REG0, 0x257);
	mmio_write32(dev->regs + THS_CTL_REG2, 0x257 << 16);

	/* Enable filter, average over 8 values. */
	mmio_write32(dev->regs + THS_FILTER_CONTROL_REG, 0x06);

	/* Enable measurement for the CPU sensor and both GPU sensors. */
	reg = mmio_read32(dev->regs + THS_CTL_REG2);
	mmio_write32(dev->regs + THS_CTL_REG2,
	             reg | BIT(0) | BIT(1) | BIT(2));

	dev->subdev_count = SENSOR_COUNT;

	return SUCCESS;
}

const struct sensor_driver sun8i_thermal_driver = {
	.drv = {
		.name  = "sun8i_thermal",
		.class = DM_CLASS_SENSOR,
		.probe = sun8i_thermal_probe,
	},
	.ops = {
		.get_info = sun8i_thermal_get_info,
		.read_raw = sun8i_thermal_read_raw,
	},
};
