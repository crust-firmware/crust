/*
 * Copyright © 2017-2019 The Crust Firmware Authors.
 * SPDX-License-Identifier: BSD-3-Clause OR GPL-2.0-only
 */

#include <counter.h>
#include <css.h>
#include <debug.h>
#include <error.h>
#include <msgbox.h>
#include <scpi.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <msgbox/sunxi-msgbox.h>
#include <platform/time.h>

#define SCPI_MEM_AREA(n) (__scpi_mem[SCPI_CLIENTS - n - 1])

#define SCPI_TX_TIMEOUT  (10 * REFCLK_KHZ) /* 10ms */

#define RX_CHAN(client)  (2 * (client))
#define TX_CHAN(client)  (2 * (client) + 1)

struct scpi_state {
	uint64_t timeout;
	bool     rx_full;
	bool     tx_full;
};

/** The shared memory area, with an address defined in the linker script. */
extern struct scpi_mem __scpi_mem[SCPI_CLIENTS];

/** The current state of each client. */
static struct scpi_state scpi_state[SCPI_CLIENTS];

/**
 * Send an SCPI message to the client, recording the timeout for when the
 * client must acknowledge the message.
 */
static void
scpi_send_message(uint8_t client)
{
	struct scpi_state *state = &scpi_state[client];
	int err;

	/* Ensure the outgoing message is fully written at this point. */
	barrier();

	/* Ensure the timeout is updated before triggering transmission. */
	state->timeout = counter_read() + SCPI_TX_TIMEOUT;
	state->tx_full = true;
	barrier();

	/* Notify the client that the message has been sent. */
	if ((err = msgbox_send(&msgbox.dev, TX_CHAN(client),
	                       SCPI_VIRTUAL_CHANNEL)))
		error("SCPI.%u: Send error: %d", client, err);
}

void
scpi_create_message(uint8_t client, uint8_t command)
{
	struct scpi_mem *mem = &SCPI_MEM_AREA(client);

	assert(!scpi_state[client].tx_full);

	/* Write the message header. */
	mem->tx_msg.command = command;
	mem->tx_msg.sender  = SCPI_SENDER_SCP;
	mem->tx_msg.size    = 0;
	mem->tx_msg.status  = SCPI_OK;

	/* Send the message. */
	scpi_send_message(client);
}

/**
 * Attempt as much forward progress as possible for each client, by checking
 * for client ACKs and then responding to incoming messages.
 *
 * Dispatch handling of messages to the appropriate function for that specific
 * command. The functions for doing so are defined in a separate file to
 * separate the API functionality from communication/state management code.
 */
void
scpi_poll(void)
{
	for (uint8_t client = 0; client < SCPI_CLIENTS; ++client) {
		struct scpi_mem *mem     = &SCPI_MEM_AREA(client);
		struct scpi_state *state = &scpi_state[client];

		/* Flush any outgoing messages. The TX buffer becomes free
		 * when the message is acknowledged or when it times out. */
		if (state->tx_full) {
			if (msgbox_last_tx_done(&msgbox.dev,
			                        TX_CHAN(client)) ||
			    counter_read() > state->timeout)
				state->tx_full = false;
		}

		/* Process incoming messages only if the TX buffer is free. */
		if (state->rx_full && !state->tx_full) {
			bool reply_needed = scpi_handle_cmd(client, mem);

			/* Acknowledge the request as soon as possible. */
			msgbox_ack_rx(&msgbox.dev, RX_CHAN(client));
			state->rx_full = false;

			if (reply_needed) {
				state->tx_full = true;
				scpi_send_message(client);
			}
		}
	}
}

void
scpi_receive_message(uint8_t client, uint32_t msg)
{
	assert(client == SCPI_CLIENT_EL2 || client == SCPI_CLIENT_EL3);

	/* Do not try to parse messages sent with a different protocol. */
	if (msg != SCPI_VIRTUAL_CHANNEL) {
		msgbox_ack_rx(&msgbox.dev, RX_CHAN(client));
		return;
	}

	/* Request handling this message as soon as the TX buffer is free. */
	scpi_state[client].rx_full = true;
}

void
scpi_init(void)
{
	int err;

	/* Secure client channel. */
	if ((err = msgbox_enable(&msgbox.dev, RX_CHAN(SCPI_CLIENT_EL3))))
		panic("SCPI.%u: Error enabling channel: %d",
		      SCPI_CLIENT_EL3, err);
	/* Non-secure client channel. */
	if ((err = msgbox_enable(&msgbox.dev, RX_CHAN(SCPI_CLIENT_EL2))))
		panic("SCPI.%u: Error enabling channel: %d",
		      SCPI_CLIENT_EL2, err);

	/* Only send the ready message once. Assume that if the system is
	 * already booted, some secondary CPUs will have been turned on. */
	if (css_get_online_cores(0) == 1)
		scpi_create_message(SCPI_CLIENT_EL3, SCPI_CMD_SCP_READY);

	info("SCPI: Initialization complete");
}
