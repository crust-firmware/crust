/*
 * Copyright © 2017-2019 The Crust Firmware Authors.
 * SPDX-License-Identifier: BSD-3-Clause OR GPL-2.0-only
 */

#ifndef COMMON_SCPI_H
#define COMMON_SCPI_H

#include <scpi_protocol.h>
#include <stdbool.h>
#include <stdint.h>

/**
 * The SCPI implementation has two clients: Linux and ATF. CPU/cluster and
 * system power state change requests are required to go through PSCI, so ATF
 * can coordinate with the secure OS (if present). These requests must only be
 * allowed if they arrive on the "secure" channel.
 */
enum {
	SCPI_CLIENT_EL3 = 0, /**< Client 0: Secure EL3 (ATF). */
	SCPI_CLIENT_EL2 = 1, /**< Client 1: Nonsec EL2 (Linux). */
	SCPI_CLIENTS,
};

/**
 * Create and send an SCPI message. This is used for commands initiated by the
 * SCP.
 *
 * @param client  The client that should receive the message.
 * @param command The command number to include in the message.
 */
void scpi_create_message(uint8_t client, uint8_t command);

/**
 * Handle a received SCPI command. This function parses the message, performs
 * any requested actions, and possibly generates a reply message.
 *
 * @param  client The client from which the message was received.
 * @param  rx_msg The received message.
 * @param  tx_msg Memory where the reply message should be placed.
 * @return If the reply message is valid and should be sent to the client.
 */
bool scpi_handle_cmd(uint8_t client, struct scpi_msg *rx_msg,
                     struct scpi_msg *tx_msg);

/**
 * Initialize the SCPI API handlers and report to the system that the firmware
 * has finished booting and is ready to accept requests.
 */
void scpi_init(void);

#endif /* COMMON_SCPI_H */
