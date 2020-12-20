/*
 * Copyright © 2020 The Crust Firmware Authors.
 * SPDX-License-Identifier: BSD-3-Clause OR GPL-2.0-only
 */

#include <error.h>
#include <mmio.h>
#include <util.h>
#include <cir/sunxi-cir.h>
#include <clock/ccu.h>
#include <gpio/sunxi-gpio.h>
#include <platform/devices.h>
#include <platform/prcm.h>

#define CIR_RXCTL  0x00
#define CIR_RXPCFG 0x10
#define CIR_RXFIFO 0x20
#define CIR_RXINT  0x2c
#define CIR_RXSTA  0x30
#define CIR_RXCFG  0x34

struct sunxi_cir_state {
	struct device_state ds;
	uint32_t            clk_stash;
	uint32_t            cfg_stash;
	uint32_t            ctl_stash;
};

static inline const struct sunxi_cir *
to_sunxi_cir(const struct device *dev)
{
	return container_of(dev, const struct sunxi_cir, dev);
}

static inline struct sunxi_cir_state *
sunxi_cir_state_for(const struct device *dev)
{
	return container_of(dev->state, struct sunxi_cir_state, ds);
}

uint32_t
sunxi_cir_poll(const struct device *dev)
{
	const struct sunxi_cir *self = to_sunxi_cir(dev);

	return mmio_get_32(self->regs + CIR_RXSTA, BIT(1));
}

static int
sunxi_cir_probe(const struct device *dev UNUSED)
{
	const struct sunxi_cir *self  = to_sunxi_cir(dev);
	struct sunxi_cir_state *state = sunxi_cir_state_for(dev);

	state->clk_stash = mmio_read_32(R_CIR_RX_CLK_REG);
	mmio_write_32(R_CIR_RX_CLK_REG, 0x80000000);

	state->cfg_stash = mmio_read_32(self->regs + CIR_RXCFG);
	mmio_write_32(self->regs + CIR_RXCFG, 0x010f0310);

	state->ctl_stash = mmio_read_32(self->regs + CIR_RXCTL);
	mmio_write_32(self->regs + CIR_RXCTL, 0x30);
	mmio_write_32(self->regs + CIR_RXCTL, 0x33);

	return SUCCESS;
}

static void
sunxi_cir_release(const struct device *dev UNUSED)
{
	const struct sunxi_cir *self  = to_sunxi_cir(dev);
	struct sunxi_cir_state *state = sunxi_cir_state_for(dev);

	mmio_write_32(R_CIR_RX_CLK_REG, state->clk_stash);
	mmio_write_32(self->regs + CIR_RXCFG, state->cfg_stash);
	mmio_write_32(self->regs + CIR_RXCTL, 0x30);
	mmio_write_32(self->regs + CIR_RXCTL, state->ctl_stash);
}

static const struct driver sunxi_cir_driver = {
	.probe   = sunxi_cir_probe,
	.release = sunxi_cir_release,
};

const struct sunxi_cir r_cir_rx = {
	.dev = {
		.name  = "r_cir_rx",
		.drv   = &sunxi_cir_driver,
		.state = &(struct sunxi_cir_state) { { 0 } }.ds,
	},
	.bus_clock = { .dev = &r_ccu.dev, .id = CLK_BUS_R_CIR },
	.mod_clock = { .dev = &r_ccu.dev, .id = CLK_R_CIR },
	.pin       = {
		.dev   = &r_pio.dev,
		.id    = SUNXI_GPIO_PIN(0, CONFIG_R_CIR_RX_PIN),
		.drive = DRIVE_10mA,
		.mode  = 2,
		.pull  = PULL_NONE,
	},
	.regs = DEV_R_CIR_RX,
};
