/*
 * Copyright © 2017-2019 The Crust Firmware Authors.
 * SPDX-License-Identifier: BSD-3-Clause OR GPL-2.0-only
 */

#include <css.h>
#include <debug.h>
#include <device.h>
#include <error.h>
#include <scpi.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <system_power.h>
#include <util.h>

enum {
	/** Replies to this command do not contain any payload data. */
	FLAG_EMPTY_PAYLOAD = BIT(0),
	/** Do not send a reply to this command. */
	FLAG_NO_REPLY      = BIT(1),
	/** Reject this command from the non-secure message box channel. */
	FLAG_SECURE_ONLY   = BIT(2),
};

struct scpi_cmd {
	/** Handler that can process a message and create a dynamic reply. */
	int             (*handler)(uint32_t *rx_payload, uint32_t *tx_payload,
	                           uint16_t *tx_size);
	/** Fixed reply payload. */
	const uint32_t *tx_payload;
	/** Expected size of received payload. */
	uint8_t         rx_size;
	/** Size of fixed reply payload, if present. */
	uint8_t         tx_size;
	/** Any combination of flags from above, if applicable. */
	uint8_t         flags;
};

static const uint8_t scpi_error_map[] = {
	SCPI_OK,        /* SUCCESS */
	SCPI_E_BUSY,    /* EBUSY */
	SCPI_E_PARAM,   /* EEXIST */
	SCPI_E_PARAM,   /* EINVAL */
	SCPI_E_DEVICE,  /* EIO */
	SCPI_E_PARAM,   /* ENODEV */
	SCPI_E_PARAM,   /* ENOENT */
	SCPI_E_SUPPORT, /* ENOTSUP */
	SCPI_E_ACCESS,  /* EPERM */
	SCPI_E_RANGE,   /* ERANGE */
};

static uint32_t scpi_map_error(int err) ATTRIBUTE(const);

static uint32_t
scpi_map_error(int err)
{
	return scpi_error_map[-err];
}

/*
 * Handler/payload data for SCPI_CMD_GET_SCP_CAP: Get SCP capability.
 */
#define SCP_FIRMWARE_VERSION(x, y, z) \
	((((x) & 0xff) << 24) | (((y) & 0xff) << 16) | ((z) & 0xffff))
#define SCPI_PAYLOAD_LIMITS(x, y)   (((x) & 0x01ff) << 16 | ((y) & 0x01ff))
#define SCPI_PROTOCOL_VERSION(x, y) (((x) & 0xffff) << 16 | ((y) & 0xffff))

static const uint32_t scpi_cmd_get_scp_cap_tx_payload[] = {
	/* SCPI protocol version. */
	SCPI_PROTOCOL_VERSION(1, 2),
	/* Payload size limits. */
	SCPI_PAYLOAD_LIMITS(SCPI_PAYLOAD_SIZE, SCPI_PAYLOAD_SIZE),
	/* Firmware version. */
	SCP_FIRMWARE_VERSION(0, 1, 9000),
	/* Commands enabled 0. */
	BIT(SCPI_CMD_SCP_READY) |
	BIT(SCPI_CMD_GET_SCP_CAP) |
	BIT(SCPI_CMD_SET_CSS_PWR) |
	BIT(SCPI_CMD_GET_CSS_PWR) |
	BIT(SCPI_CMD_SET_SYS_PWR),
	/* Commands enabled 1. */
	0,
	/* Commands enabled 2. */
	0,
	/* Commands enabled 3. */
	0,
};

/*
 * Handler/payload data for SCPI_CMD_SET_CSS_PWR: Set CSS power state.
 *
 * This sets the power state of a single core, its parent cluster, and the CSS.
 *
 * The power state provided by PSCI is already coordinated. Simply turn things
 * on from highest to lowest power level, then turn things off from lowest to
 * highest power level. This ensures no power domain is turned on before its
 * parent, and no power domain is turned off before any of its children.
 */
static int
scpi_cmd_set_css_pwr_handler(uint32_t *rx_payload,
                             uint32_t *tx_payload UNUSED,
                             uint16_t *tx_size UNUSED)
{
	uint32_t descriptor    = rx_payload[0];
	uint8_t  core          = (descriptor >> 0x00) & GENMASK(3, 0);
	uint8_t  cluster       = (descriptor >> 0x04) & GENMASK(3, 0);
	uint8_t  core_state    = (descriptor >> 0x08) & GENMASK(3, 0);
	uint8_t  cluster_state = (descriptor >> 0x0c) & GENMASK(3, 0);
	uint8_t  css_state     = (descriptor >> 0x10) & GENMASK(3, 0);
	int err;

	/* Do not check if the CSS should be turned on, as receiving this
	 * command from an ARM CPU via PSCI implies that it is already on. */
	if (cluster_state == SCPI_CSS_ON &&
	    (err = css_set_cluster_state(cluster, cluster_state)))
		return err;
	if ((err = css_set_core_state(cluster, core, core_state)))
		return err;
	if (cluster_state != SCPI_CSS_ON &&
	    (err = css_set_cluster_state(cluster, cluster_state)))
		return err;
	if (css_state != SCPI_CSS_ON &&
	    (err = css_set_css_state(css_state)))
		return err;
	/* Turning everything off means system suspend. */
	if (css_state == SCPI_CSS_OFF)
		system_suspend();

	return SUCCESS;
}

/*
 * Handler/payload data for SCPI_CMD_GET_CSS_PWR: Get CSS power state.
 *
 * This gets the power states of all clusters and all cores they contain.
 */
#define CLUSTER_ID(x)          ((x) & GENMASK(3, 0))
#define CLUSTER_POWER_STATE(x) (((x) & GENMASK(3, 0)) << 4)
#define CORE_POWER_STATES(x)   ((x) << 8)
static int
scpi_cmd_get_css_pwr_handler(uint32_t *rx_payload UNUSED,
                             uint32_t *tx_payload, uint16_t *tx_size)
{
	uint8_t  clusters = css_get_cluster_count();
	uint16_t descriptor;

	/* Each cluster has its own power state descriptor. */
	for (uint8_t i = 0; i < clusters; ++i) {
		descriptor = CLUSTER_ID(i) |
		             CLUSTER_POWER_STATE(css_get_cluster_state(i)) |
		             CORE_POWER_STATES(css_get_online_cores(i));
		/* Work around the hardware byte swapping, since this is an
		 * array of elements each aligned to less than 4 bytes. */
		((uint16_t *)tx_payload)[i ^ 1] = descriptor;
	}
	*tx_size = clusters * sizeof(descriptor);

	return SUCCESS;
}

/*
 * Handler/payload data for SCPI_CMD_SET_SYS_PWR: Set system power state.
 */
static int
scpi_cmd_set_sys_power_handler(uint32_t *rx_payload,
                               uint32_t *tx_payload UNUSED,
                               uint16_t *tx_size UNUSED)
{
	uint8_t state = rx_payload[0];

	if (state == SCPI_SYSTEM_SHUTDOWN)
		system_shutdown();
	else if (state == SCPI_SYSTEM_REBOOT || state == SCPI_SYSTEM_RESET)
		system_reset();
	else
		return EINVAL;

	return SUCCESS;
}

/*
 * The list of supported SCPI commands.
 */
static const struct scpi_cmd scpi_cmds[] = {
	[SCPI_CMD_SCP_READY] = {
		.flags = FLAG_EMPTY_PAYLOAD | FLAG_NO_REPLY | FLAG_SECURE_ONLY,
	},
	[SCPI_CMD_GET_SCP_CAP] = {
		.tx_payload = scpi_cmd_get_scp_cap_tx_payload,
		.tx_size    = sizeof(scpi_cmd_get_scp_cap_tx_payload),
	},
	[SCPI_CMD_SET_CSS_PWR] = {
		.handler = scpi_cmd_set_css_pwr_handler,
		.rx_size = sizeof(uint32_t),
		.flags   = FLAG_NO_REPLY | FLAG_SECURE_ONLY,
	},
	[SCPI_CMD_GET_CSS_PWR] = {
		.handler = scpi_cmd_get_css_pwr_handler,
	},
	[SCPI_CMD_SET_SYS_PWR] = {
		.handler = scpi_cmd_set_sys_power_handler,
		.rx_size = sizeof(uint8_t),
		.flags   = FLAG_SECURE_ONLY,
	},
};

/*
 * Generic SCPI command handling function.
 */
bool
scpi_handle_cmd(uint8_t client, struct scpi_mem *mem)
{
	struct scpi_msg *rx_msg = &mem->rx_msg;
	struct scpi_msg *tx_msg = &mem->tx_msg;
	const struct scpi_cmd *cmd;

	/* Initialize the response (defaults for unsupported commands). */
	tx_msg->command = rx_msg->command;
	tx_msg->sender  = rx_msg->sender;
	tx_msg->size    = 0;
	tx_msg->status  = SCPI_E_SUPPORT;

	/* Avoid reading past the end of the array; reply with the error. */
	if (rx_msg->command >= ARRAY_SIZE(scpi_cmds))
		return true;
	cmd = &scpi_cmds[rx_msg->command];

	/* Update the command status and payload based on the message. */
	if ((cmd->flags & FLAG_SECURE_ONLY) && client != SCPI_CLIENT_EL3) {
		/* Prevent Linux from sending commands that bypass PSCI. */
		tx_msg->status = SCPI_E_ACCESS;
	} else if (rx_msg->size != cmd->rx_size) {
		/* Check that the request payload matches the expected size. */
		tx_msg->status = SCPI_E_SIZE;
	} else if (cmd->flags & FLAG_EMPTY_PAYLOAD) {
		/* Some reply messages do not need an additional payload. */
		tx_msg->status = SCPI_OK;
	} else if (cmd->tx_payload != NULL) {
		/* Use a fixed reply payload, if present. */
		tx_msg->size   = cmd->tx_size;
		tx_msg->status = SCPI_OK;
		memcpy(&tx_msg->payload, cmd->tx_payload, cmd->tx_size);
	} else if (cmd->handler) {
		/* Run the handler for this command to make a response. */
		int err = cmd->handler(rx_msg->payload, tx_msg->payload,
		                       &tx_msg->size);
		tx_msg->status = scpi_map_error(err);
	} else {
		warn("SCPI: Unknown command %u", rx_msg->command);
	}

	/* Report back if a reply should be sent. */
	return !(cmd->flags & FLAG_NO_REPLY);
}
