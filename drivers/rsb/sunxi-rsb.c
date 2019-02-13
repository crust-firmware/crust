/*
 * Copyright © 2017-2019 The Crust Firmware Authors.
 * SPDX-License-Identifier: BSD-3-Clause OR GPL-2.0-only
 */

#include <dm.h>
#include <error.h>
#include <mmio.h>
#include <rsb.h>
#include <clock/sunxi-ccu.h>
#include <gpio/sunxi-gpio.h>
#include <irqchip/sun4i-intc.h>
#include <rsb/sunxi-rsb.h>
#include <platform/devices.h>

#define RSB_CTRL_REG   0x00
#define RSB_CCR_REG    0x04
#define RSB_INT_EN_REG 0x08
#define RSB_STAT_REG   0x0c
#define RSB_ADDR_REG   0x10
#define RSB_DLEN_REG   0x18
#define RSB_DATA_REG   0x1c
#define RSB_LCR_REG    0x24
#define RSB_PMCR_REG   0x28
#define RSB_CMD_REG    0x2c
#define RSB_SADDR_REG  0x30

#define I2C_BCAST_ADDR 0

static int sunxi_rsb_set_rate(struct device *dev, uint32_t rate);

static int
sunxi_rsb_do_command(struct device *dev, uint32_t addr, uint32_t cmd)
{
	mmio_write_32(dev->regs + RSB_CMD_REG, cmd);
	mmio_write_32(dev->regs + RSB_SADDR_REG, addr);
	mmio_write_32(dev->regs + RSB_CTRL_REG, BIT(7));

	mmio_pollz_32(dev->regs + RSB_CTRL_REG, BIT(7));

	return mmio_read_32(dev->regs + RSB_STAT_REG) == BIT(0)
	       ? SUCCESS : EIO;
}

static int
sunxi_rsb_init_pmic(struct device *dev, uint32_t addr, uint8_t reg,
                    uint8_t data)
{
	int err;

	/* Switch the PMIC to RSB mode. */
	mmio_write_32(dev->regs + RSB_PMCR_REG,
	              BIT(31) | data << 16 | reg << 8 | I2C_BCAST_ADDR);
	mmio_pollz_32(dev->regs + RSB_PMCR_REG, BIT(31));

	/* Raise the clock to 3MHz. */
	if ((err = sunxi_rsb_set_rate(dev, 3000000)))
		return err;

	/* Set the PMIC's runtime address. */
	return sunxi_rsb_do_command(dev, addr, RSB_SRTA);
}

static int
sunxi_rsb_read(struct device *dev, uint8_t addr, uint8_t reg, uint8_t *data)
{
	int err;

	mmio_write_32(dev->regs + RSB_ADDR_REG, reg);

	if ((err = sunxi_rsb_do_command(dev, RSB_RTADDR(addr), RSB_RD8)))
		return err;

	*data = mmio_read_32(dev->regs + RSB_DATA_REG);

	return SUCCESS;
}

static int
sunxi_rsb_set_rate(struct device *dev, uint32_t rate)
{
	struct clock_handle *clock = &dev->clocks[0];
	uint32_t dev_rate;
	uint8_t  divider;
	int err;

	if ((err = clock_get_rate(clock->dev, clock->id, &dev_rate)))
		return err;

	dev_rate /= 2;
	if (rate == 0 || rate < dev_rate / 256 || rate > dev_rate)
		return ERANGE;

	divider = dev_rate / rate - 1;
	mmio_write_32(dev->regs + RSB_CCR_REG, 1U << 8 | divider);

	return SUCCESS;
}

static int
sunxi_rsb_write(struct device *dev, uint8_t addr, uint8_t reg, uint8_t data)
{
	mmio_write_32(dev->regs + RSB_ADDR_REG, reg);
	mmio_write_32(dev->regs + RSB_DATA_REG, data);

	return sunxi_rsb_do_command(dev, RSB_RTADDR(addr), RSB_WR8);
}

static int
sunxi_rsb_probe(struct device *dev)
{
	int err;

	if ((err = dm_setup_clocks(dev, 1)))
		return err;

	if ((err = dm_setup_pins(dev, RSB_NUM_PINS)))
		return err;

	mmio_write_32(dev->regs + RSB_CTRL_REG, BIT(0));
	mmio_pollz_32(dev->regs + RSB_CTRL_REG, BIT(0));

	/* Set the bus clock to a rate also compatible with I²C. */
	if ((err = sunxi_rsb_set_rate(dev, 400000)))
		return err;

	return SUCCESS;
}

static const struct rsb_driver sunxi_rsb_driver = {
	.drv = {
		.class = DM_CLASS_RSB,
		.probe = sunxi_rsb_probe,
	},
	.ops = {
		.init_pmic = sunxi_rsb_init_pmic,
		.read      = sunxi_rsb_read,
		.set_rate  = sunxi_rsb_set_rate,
		.write     = sunxi_rsb_write,
	},
};

struct device r_rsb __device = {
	.name   = "r_rsb",
	.regs   = DEV_R_RSB,
	.drv    = &sunxi_rsb_driver.drv,
	.clocks = CLOCK_PARENT(r_ccu, R_CCU_CLOCK_R_RSB),
	.irq    = IRQ_HANDLE {
		.dev = &r_intc,
		.irq = IRQ_R_RSB,
	},
	.pins = GPIO_PINS(RSB_NUM_PINS) {
		{ &r_pio, SUNXI_GPIO_PIN(0, 0), 2 },
		{ &r_pio, SUNXI_GPIO_PIN(0, 1), 2 },
	},
};
