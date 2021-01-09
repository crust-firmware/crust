/*
 * Copyright © 2020-2021 The Crust Firmware Authors.
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

#include "cir.h"

#define CIR_RXCTL  0x00
#define CIR_RXPCFG 0x10
#define CIR_RXFIFO 0x20
#define CIR_RXINT  0x2c
#define CIR_RXSTA  0x30
#define CIR_RXCFG  0x34

struct sunxi_cir_state {
	struct device_state ds;
	struct cir_dec_ctx  dec_ctx;
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
	const struct sunxi_cir *self  = to_sunxi_cir(dev);
	struct sunxi_cir_state *state = sunxi_cir_state_for(dev);
	struct cir_dec_ctx *dec_ctx   = &state->dec_ctx;

	/* Feed the decoder data as needed and as it becomes available. */
	if (dec_ctx->width <= 0) {
		/* If no data is available, do not call the decoder. */
		if (!(mmio_read_32(self->regs + CIR_RXSTA) >> 8))
			return 0;

		uint32_t sample = mmio_read_32(self->regs + CIR_RXFIFO);
		dec_ctx->pulse = sample >> 7;
		dec_ctx->width = sample & GENMASK(6, 0);
	}

	return cir_decode(dec_ctx);
}

static int
sunxi_cir_probe(const struct device *dev)
{
	const struct sunxi_cir *self = to_sunxi_cir(dev);
	int err;

	/* Set module clock parent and divider. */
	mmio_write_32(R_CIR_RX_CLK_REG, 0x0);

	if ((err = clock_get(&self->bus_clock)))
		return err;
	if ((err = clock_get(&self->mod_clock)))
		goto err_put_bus_clock;
	if ((err = gpio_get(&self->pin)))
		goto err_put_mod_clock;

	/* Configure thresholds and sample clock. */
	mmio_write_32(self->regs + CIR_RXCFG, 0x010f0310);

	/* Enable CIR module. */
	mmio_write_32(self->regs + CIR_RXCTL, 0x33);

	return SUCCESS;

err_put_mod_clock:
	clock_put(&self->mod_clock);
err_put_bus_clock:
	clock_put(&self->bus_clock);

	return err;
}

static void
sunxi_cir_release(const struct device *dev)
{
	const struct sunxi_cir *self = to_sunxi_cir(dev);

	gpio_put(&self->pin);
	clock_put(&self->mod_clock);
	clock_put(&self->bus_clock);
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
