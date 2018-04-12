/*
 * Copyright © 2017-2018 The Crust Firmware Authors.
 * SPDX-License-Identifier: (BSD-3-Clause OR GPL-2.0)
 */

#include <bitmap.h>
#include <clock.h>
#include <css.h>
#include <debug.h>
#include <dm.h>
#include <dvfs.h>
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
	uint32_t  (*handler)(uint32_t *rx_payload, uint32_t *tx_payload,
	                     uint16_t *tx_size);
	/** Fixed reply payload. */
	uint32_t *tx_payload;
	/** Expected size of received payload. */
	uint16_t  rx_size;
	/** Size of fixed reply payload, if present. */
	uint16_t  tx_size;
	/** Any combination of flags from above, if applicable. */
	uint8_t   flags;
};

static const uint32_t scpi_error_map[] = {
	SCPI_OK,        /* SUCCESS */
	SCPI_E_BUSY,    /* EBUSY */
	SCPI_E_PARAM,   /* EEXIST */
	SCPI_E_PARAM,   /* EINVAL */
	SCPI_E_DEVICE,  /* EIO */
	SCPI_E_SUPPORT, /* ENODEV */
	SCPI_E_SUPPORT, /* ENOTSUP */
	SCPI_E_ACCESS,  /* EPERM */
	SCPI_E_RANGE,   /* ERANGE */
};

static uint32_t __const
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

static uint32_t scpi_cmd_get_scp_cap_tx_payload[] = {
	/* SCPI protocol version. */
	SCPI_PROTOCOL_VERSION(1, 2),
	/* Payload size limits. */
	SCPI_PAYLOAD_LIMITS(SCPI_PAYLOAD_SIZE, SCPI_PAYLOAD_SIZE),
	/* Firmware version. */
	SCP_FIRMWARE_VERSION(0, 0, 0),
	/* Commands enabled 0. */
	BITMAP_BIT(SCPI_CMD_SCP_READY) |
	BITMAP_BIT(SCPI_CMD_GET_SCP_CAP) |
	BITMAP_BIT(SCPI_CMD_SET_CSS_PWR) |
	BITMAP_BIT(SCPI_CMD_GET_CSS_PWR) |
	BITMAP_BIT(SCPI_CMD_SET_SYS_PWR) |
	BITMAP_BIT(SCPI_CMD_GET_DVFS_CAP) |
	BITMAP_BIT(SCPI_CMD_GET_DVFS_INFO) |
	BITMAP_BIT(SCPI_CMD_SET_DVFS) |
	BITMAP_BIT(SCPI_CMD_GET_DVFS) |
	BITMAP_BIT(SCPI_CMD_GET_CLOCK_CAP) |
	BITMAP_BIT(SCPI_CMD_GET_CLOCK_INFO) |
	BITMAP_BIT(SCPI_CMD_SET_CLOCK) |
	BITMAP_BIT(SCPI_CMD_GET_CLOCK),
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
uint32_t
scpi_cmd_set_css_pwr_handler(uint32_t *rx_payload,
                             uint32_t *tx_payload __unused,
                             uint16_t *tx_size __unused)
{
	uint32_t descriptor    = rx_payload[0];
	uint8_t  core          = (descriptor >> 0x00) & BITMASK(0, 4);
	uint8_t  cluster       = (descriptor >> 0x04) & BITMASK(0, 4);
	uint8_t  core_state    = (descriptor >> 0x08) & BITMASK(0, 4);
	uint8_t  cluster_state = (descriptor >> 0x0c) & BITMASK(0, 4);
	uint8_t  css_state     = (descriptor >> 0x10) & BITMASK(0, 4);

	/* Do not check if the CSS should be turned on, as receiving this
	 * command from an ARM CPU via PSCI implies that it is already on. */
	if (cluster_state == POWER_STATE_ON &&
	    css_set_cluster_state(cluster, cluster_state))
		return SCPI_E_DEVICE;
	if (css_set_core_state(cluster, core, core_state))
		return SCPI_E_DEVICE;
	if (cluster_state != POWER_STATE_ON &&
	    css_set_cluster_state(cluster, cluster_state))
		return SCPI_E_DEVICE;
	if (css_state != POWER_STATE_ON &&
	    css_set_css_state(css_state))
		return SCPI_E_DEVICE;
	/* Turning everything off means system suspend. */
	if (css_state == POWER_STATE_OFF)
		system_suspend();

	return SCPI_OK;
}

/*
 * Handler/payload data for SCPI_CMD_GET_CSS_PWR: Get CSS power state.
 *
 * This gets the power states of all clusters and all cores they contain.
 */
#define CLUSTER_ID(x)          ((x) & BITMASK(0, 4))
#define CLUSTER_POWER_STATE(x) (((x) & BITMASK(0, 4)) << 4)
#define CORE_POWER_STATES(x)   ((x) << 8)
uint32_t
scpi_cmd_get_css_pwr_handler(uint32_t *rx_payload __unused,
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

	return SCPI_OK;
}

/*
 * Handler/payload data for SCPI_CMD_SET_SYS_PWR: Set system power state.
 */
uint32_t
scpi_cmd_set_sys_power_handler(uint32_t *rx_payload,
                               uint32_t *tx_payload __unused,
                               uint16_t *tx_size __unused)
{
	uint8_t system_state = rx_payload[0] & BITMASK(0, 8);

	if (system_state == SYSTEM_POWER_STATE_REBOOT ||
	    system_state == SYSTEM_POWER_STATE_RESET)
		system_reset();

	if (system_state != SYSTEM_POWER_STATE_SHUTDOWN)
		return SCPI_E_PARAM;

	system_shutdown();

	return SCPI_OK;
}

/*
 * Handler/payload data for SCPI_CMD_GET_DVFS_CAP: Get DVFS capability.
 */
uint32_t
scpi_cmd_get_dvfs_cap_handler(uint32_t *rx_payload __unused,
                              uint32_t *tx_payload, uint16_t *tx_size)
{
	tx_payload[0] = dm_count_subdevs_by_class(DM_CLASS_DVFS);
	*tx_size      = 1;

	return SCPI_OK;
}

/*
 * Handler/payload data for SCPI_CMD_GET_DVFS_INFO: Get DVFS info.
 */
uint32_t
scpi_cmd_get_dvfs_info_handler(uint32_t *rx_payload, uint32_t *tx_payload,
                               uint16_t *tx_size)
{
	struct device    *dev;
	struct dvfs_info *info;
	uint8_t id;
	uint8_t index = rx_payload[0] & BITMASK(0, 8);

	if ((dev = dm_get_subdev_by_index(DM_CLASS_DVFS, index, &id)) == NULL)
		return SCPI_E_PARAM;
	if ((info = dvfs_get_info(dev, id)) == NULL)
		return SCPI_E_PARAM;

	tx_payload[0] = index | info->opp_count << 8 | info->latency << 16;
	for (uint8_t i = 0; i < info->opp_count; ++i) {
		tx_payload[2 * i + 1] = info->opps[i].rate * 1000000;
		tx_payload[2 * i + 2] = info->opps[i].voltage;
	}
	*tx_size = sizeof(uint32_t) * (2 * info->opp_count + 1);

	assert(*tx_size <= SCPI_PAYLOAD_SIZE);

	return SCPI_OK;
}

/*
 * Handler/payload data for SCPI_CMD_SET_DVFS: Set DVFS.
 */
uint32_t
scpi_cmd_set_dvfs_handler(uint32_t *rx_payload, uint32_t *tx_payload __unused,
                          uint16_t *tx_size __unused)
{
	struct device *dev;
	uint8_t id;
	uint8_t index = (rx_payload[0] >> 0) & BITMASK(0, 8);
	uint8_t opp   = (rx_payload[0] >> 8) & BITMASK(0, 8);

	if ((dev = dm_get_subdev_by_index(DM_CLASS_DVFS, index, &id)) == NULL)
		return SCPI_E_PARAM;

	return scpi_map_error(dvfs_set_opp(dev, id, opp));
}

/*
 * Handler/payload data for SCPI_CMD_GET_DVFS: Get DVFS.
 */
uint32_t
scpi_cmd_get_dvfs_handler(uint32_t *rx_payload, uint32_t *tx_payload,
                          uint16_t *tx_size)
{
	struct device *dev;
	uint8_t id;
	uint8_t index = rx_payload[0] & BITMASK(0, 8);

	if ((dev = dm_get_subdev_by_index(DM_CLASS_DVFS, index, &id)) == NULL)
		return SCPI_E_PARAM;

	tx_payload[0] = dvfs_get_opp(dev, id);
	*tx_size      = sizeof(uint8_t);

	return SCPI_OK;
}

/*
 * Handler/payload data for SCPI_CMD_GET_CLOCK_CAP: Get clock capability.
 */
uint32_t
scpi_cmd_get_clock_cap_handler(uint32_t *rx_payload __unused,
                               uint32_t *tx_payload, uint16_t *tx_size)
{
	tx_payload[0] = dm_count_subdevs_by_class(DM_CLASS_CLOCK);
	*tx_size      = 2;

	return SCPI_OK;
}

/*
 * Handler/payload data for SCPI_CMD_GET_CLOCK_INFO: Get clock info.
 */
uint32_t
scpi_cmd_get_clock_info_handler(uint32_t *rx_payload, uint32_t *tx_payload,
                                uint16_t *tx_size)
{
	struct clock_info *info;
	struct device     *dev;
	uint8_t id;
	uint8_t index = rx_payload[0] & BITMASK(0, 8);

	if ((dev = dm_get_subdev_by_index(DM_CLASS_CLOCK, index, &id)) == NULL)
		return SCPI_E_PARAM;
	if ((info = clock_get_info(dev, id)) == NULL)
		return SCPI_E_PARAM;

	tx_payload[0] = index | (info->flags & CLK_SCPI_MASK) << 16;
	tx_payload[1] = info->min_rate;
	tx_payload[2] = info->max_rate;
	strncpy((char *)&tx_payload[3], info->name, 20);
	*tx_size = 32;

	return SCPI_OK;
}

/*
 * Handler/payload data for SCPI_CMD_SET_CLOCK: Set clock value.
 */
uint32_t
scpi_cmd_set_clock_handler(uint32_t *rx_payload, uint32_t *tx_payload __unused,
                           uint16_t *tx_size __unused)
{
	struct clock_info *info;
	struct device     *dev;
	uint8_t  id;
	uint8_t  index = rx_payload[0] & BITMASK(0, 8);
	uint32_t rate  = rx_payload[1];

	if ((dev = dm_get_subdev_by_index(DM_CLASS_CLOCK, index, &id)) == NULL)
		return SCPI_E_PARAM;
	if ((info = clock_get_info(dev, id)) == NULL)
		return SCPI_E_PARAM;
	if (!(info->flags & CLK_WRITABLE))
		return SCPI_E_ACCESS;

	return scpi_map_error(clock_set_rate(dev, id, rate));
}

/*
 * Handler/payload data for SCPI_CMD_GET_CLOCK: Get clock value.
 */
uint32_t
scpi_cmd_get_clock_handler(uint32_t *rx_payload, uint32_t *tx_payload,
                           uint16_t *tx_size)
{
	struct clock_info *info;
	struct device     *dev;
	int      err;
	uint8_t  id;
	uint8_t  index = rx_payload[0] & BITMASK(0, 8);
	uint32_t rate;

	if ((dev = dm_get_subdev_by_index(DM_CLASS_CLOCK, index, &id)) == NULL)
		return SCPI_E_PARAM;
	if ((info = clock_get_info(dev, id)) == NULL)
		return SCPI_E_PARAM;
	if (!(info->flags & CLK_READABLE))
		return SCPI_E_ACCESS;
	if ((err = clock_get_rate(dev, id, &rate)))
		return scpi_map_error(err);

	tx_payload[0] = rate;
	*tx_size      = sizeof(rate);

	return SCPI_OK;
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
	[SCPI_CMD_GET_DVFS_CAP] = {
		.handler = scpi_cmd_get_dvfs_cap_handler,
	},
	[SCPI_CMD_GET_DVFS_INFO] = {
		.handler = scpi_cmd_get_dvfs_info_handler,
		.rx_size = sizeof(uint8_t),
	},
	[SCPI_CMD_SET_DVFS] = {
		.handler = scpi_cmd_set_dvfs_handler,
		.rx_size = sizeof(uint16_t),
	},
	[SCPI_CMD_GET_DVFS] = {
		.handler = scpi_cmd_get_dvfs_handler,
		.rx_size = sizeof(uint8_t),
	},
	[SCPI_CMD_GET_CLOCK_CAP] = {
		.handler = scpi_cmd_get_clock_cap_handler,
	},
	[SCPI_CMD_GET_CLOCK_INFO] = {
		.handler = scpi_cmd_get_clock_info_handler,
		.rx_size = sizeof(uint16_t),
	},
	[SCPI_CMD_SET_CLOCK] = {
		.handler = scpi_cmd_set_clock_handler,
		.rx_size = 2 * sizeof(uint32_t),
	},
	[SCPI_CMD_GET_CLOCK] = {
		.handler = scpi_cmd_get_clock_handler,
		.rx_size = sizeof(uint16_t),
	},
};

/*
 * Generic SCPI command handling function.
 */
bool
scpi_handle_cmd(uint8_t client, struct scpi_msg *rx_msg,
                struct scpi_msg *tx_msg)
{
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
	if ((cmd->flags & FLAG_SECURE_ONLY) && client != SCPI_CLIENT_SECURE) {
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
		tx_msg->status = cmd->handler(rx_msg->payload, tx_msg->payload,
		                              &tx_msg->size);
	} else {
		warn("SCPI: Unknown command %u", rx_msg->command);
	}

	/* Report back if a reply should be sent. */
	return !(cmd->flags & FLAG_NO_REPLY);
}
