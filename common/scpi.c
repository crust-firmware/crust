/*
 * Copyright © 2017-2020 The Crust Firmware Authors.
 * SPDX-License-Identifier: BSD-3-Clause OR GPL-2.0-only
 */

#include <counter.h>
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
	bool     tx_full;
};

static const struct device *mailbox;

/** The shared memory area, with an address defined in the linker script. */
extern struct scpi_mem __scpi_mem[SCPI_CLIENTS];

/** The current state of each client. */
static struct scpi_state scpi_state[SCPI_CLIENTS];

/**
 * Send an SCPI message to the client, recording the timeout for when the
 * client must acknowledge the message.
 */
static void
scpi_send_message(uint8_t client, struct scpi_state *state)
{
	int err;

	/* Ensure the outgoing message is fully written at this point. */
	barrier();

	/* Ensure the timeout is updated before triggering transmission. */
	state->timeout = counter_read() + SCPI_TX_TIMEOUT;
	state->tx_full = true;
	barrier();

	/* Notify the client that the message has been sent. */
	if ((err = msgbox_send(mailbox, TX_CHAN(client),
	                       SCPI_VIRTUAL_CHANNEL)))
		error("SCPI.%u: Send error: %d", client, err);
}

void
scpi_create_message(uint8_t client, uint8_t command)
{
	struct scpi_mem *mem     = &SCPI_MEM_AREA(client);
	struct scpi_state *state = &scpi_state[client];

	if (!mailbox || state->tx_full)
		return;

	/* Write the message header. */
	mem->tx_msg.command = command;
	mem->tx_msg.sender  = SCPI_SENDER_SCP;
	mem->tx_msg.size    = 0;
	mem->tx_msg.status  = SCPI_OK;

	/* Send the message. */
	scpi_send_message(client, state);
}

/**
 * Attempt as much forward progress as possible for a client, by checking
 * for client ACKs and then responding to incoming messages.
 *
 * Dispatch handling of messages to the appropriate function for that specific
 * command. The functions for doing so are defined in a separate file to
 * separate the API functionality from communication/state management code.
 */
static void
scpi_poll_one_client(uint8_t client)
{
	struct scpi_state *state = &scpi_state[client];
	uint8_t rx_chan = RX_CHAN(client);
	uint8_t tx_chan = TX_CHAN(client);

	/* Flush any outgoing messages. The TX buffer becomes free when a
	 * previously-sent message is acknowledged or when it times out. */
	if (state->tx_full) {
		if (msgbox_last_tx_done(mailbox, tx_chan) ||
		    counter_read() > state->timeout)
			state->tx_full = false;
	}

	/* Once the TX buffer is free, we can process new messages, reading
	 * from the RX buffer and generating responses in the TX buffer. */
	if (!state->tx_full) {
		bool reply_needed = false;
		uint32_t msg;

		/* Try to grab a new message. All errors are handled by
		 * retrying on the next iteration through the main loop. */
		if (msgbox_receive(mailbox, rx_chan, &msg) == SUCCESS) {
			/* Only process messages sent with the correct
			 * protocol, which SCPI calls a "virtual channel". */
			if (msg == SCPI_VIRTUAL_CHANNEL) {
				struct scpi_mem *mem = &SCPI_MEM_AREA(client);

				/* The handler relays if a reply is needed. */
				reply_needed = scpi_handle_cmd(client, mem);
			}

			/* Acknowledging the message allows the client to reuse
			 * the RX buffer, so the handler must run first. */
			msgbox_ack_rx(mailbox, rx_chan);
		}

		/* If the TX buffer now contains a reply, send it. */
		if (reply_needed)
			scpi_send_message(client, state);
	}
}

void
scpi_init(void)
{
	mailbox = device_get_or_null(&msgbox.dev);
}

void
scpi_poll(void)
{
	/* Do nothing if there is no mailbox available. */
	if (!mailbox)
		return;

	for (uint8_t client = 0; client < SCPI_CLIENTS; ++client)
		scpi_poll_one_client(client);
}

void
scpi_exit(void)
{
	/* Drop the reference so the clock can be turned off in suspend. */
	device_put(mailbox);
	mailbox = NULL;
}
