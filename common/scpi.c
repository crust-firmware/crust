/*
 * Copyright © 2017-2019 The Crust Firmware Authors.
 * SPDX-License-Identifier: BSD-3-Clause OR GPL-2.0-only
 */

#include <compiler.h>
#include <css.h>
#include <debug.h>
#include <error.h>
#include <interrupts.h>
#include <msgbox.h>
#include <scpi.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <util.h>
#include <wallclock.h>
#include <msgbox/sunxi-msgbox.h>
#include <platform/memory.h>
#include <platform/time.h>

#define SCPI_MEM_AREA(n) (__scpi_mem[SCPI_CLIENTS - n - 1])

#define SCPI_TX_TIMEOUT  (10 * REFCLK_KHZ) /* 10ms */

#define RX_CHAN(client)  (2 * (client))
#define TX_CHAN(client)  (2 * (client) + 1)

/** The shared memory area, with an address defined in the linker script. */
extern struct scpi_mem __scpi_mem[SCPI_CLIENTS];

/** The time at which the message sent to this client is considered lost. */
static uint64_t scpi_timeout[SCPI_CLIENTS];

/**
 * Wait for a previous transmission to be acknowledged, with a timeout.
 *
 * This function acquires the shared memory buffer.
 */
static void
scpi_wait_tx_done(uint8_t client)
{
	while (wallclock_read() < scpi_timeout[client])
		if (msgbox_last_tx_done(&msgbox, TX_CHAN(client)))
			break;
	/* Prevent reordering shared memory reads before the loop. */
	barrier();
}

/**
 * Part 3 of SCPI message handling.
 *
 * This function releases the shared memory buffer.
 */
static void
scpi_send_message(uint8_t client)
{
	int err;

	/* Ensure that the outgoing message is fully written at this point. */
	barrier();

	/* Ensure that the timeout is updated before transmission. */
	scpi_timeout[client] = wallclock_read() + SCPI_TX_TIMEOUT;
	barrier();

	/* Notify the client that the message has been sent. */
	if ((err = msgbox_send(&msgbox, TX_CHAN(client),
	                       SCPI_VIRTUAL_CHANNEL)))
		error("SCPI.%u: Send error: %d", client, err);
}

/**
 * Parts 1 & 2 of SCPI message handling (SCP-initiated messages).
 *
 * This function writes a message directly to the client's shared memory for
 * transmission. It must disable interrupts to prevent an incoming SCPI command
 * from generating a reply that would overwrite this message.
 */
void
scpi_create_message(uint8_t client, uint8_t command)
{
	struct scpi_mem *mem = &SCPI_MEM_AREA(client);
	uint32_t flags       = disable_interrupts();

	/* Try to ensure the TX buffer is free before writing to it. */
	scpi_wait_tx_done(client);

	/* Create the message header. */
	mem->tx_msg.command = command;
	mem->tx_msg.sender  = SCPI_SENDER_SCP;
	mem->tx_msg.size    = 0;
	mem->tx_msg.status  = SCPI_OK;

	scpi_send_message(client);
	restore_interrupts(flags);
}

/**
 * Part 2 of SCPI message handling (received messages).
 *
 * Dispatch handling of the message to the appropriate function for each
 * command. The functions for doing so are defined in a separate file to
 * separate the API functionality from communication/synchronization code.
 *
 * When the command's actions are complete, and a reply message is created,
 * enqueue a function to send the response.
 */
static void
scpi_handle_message(uint8_t client)
{
	struct scpi_mem *mem = &SCPI_MEM_AREA(client);

	/* Try to ensure the TX buffer is free before writing to it. */
	scpi_wait_tx_done(client);

	/* Handle the command, and send the reply if one is generated. */
	if (scpi_handle_cmd(client, &mem->rx_msg, &mem->tx_msg))
		scpi_send_message(client);
}

/**
 * Part 1 of SCPI message handling (received messages).
 *
 * This is the callback from the message box framework; it is called when a new
 * message is received.
 */
void
scpi_receive_message(uint8_t client, uint32_t msg)
{
	assert(client == SCPI_CLIENT_EL2 || client == SCPI_CLIENT_EL3);

	/* Do not try to parse messages sent with a different protocol. */
	if (msg != SCPI_VIRTUAL_CHANNEL)
		return;

	scpi_handle_message(client);
}

void
scpi_init(void)
{
	int err;

	/* Non-secure client channel. */
	if ((err = msgbox_enable(&msgbox, RX_CHAN(SCPI_CLIENT_EL3))))
		panic("SCPI.%u: Error enabling channel: %d",
		      SCPI_CLIENT_EL3, err);
	/* Secure client channel. */
	if ((err = msgbox_enable(&msgbox, RX_CHAN(SCPI_CLIENT_EL2))))
		panic("SCPI.%u: Error enabling channel: %d",
		      SCPI_CLIENT_EL2, err);

	/* Only send the ready message once. Assume that if the system is
	 * already booted, some secondary CPUs will have been turned on. */
	if (css_get_online_cores(0) == 1)
		scpi_create_message(SCPI_CLIENT_EL3, SCPI_CMD_SCP_READY);

	info("SCPI: Initialization complete");
}
