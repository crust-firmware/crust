/*
 * Copyright © 2017-2018 The Crust Firmware Authors.
 * SPDX-License-Identifier: BSD-3-Clause OR GPL-2.0-only
 */

#include <compiler.h>
#include <debug.h>
#include <dm.h>
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
#include <work.h>
#include <platform/memory.h>
#include <platform/time.h>

#define SCPI_BUFFER_MAX  2

#define SCPI_MEM_AREA(n) (((struct scpi_mem *)SCPI_MEM_BASE)[n])

#define SCPI_TX_TIMEOUT  (100 * REFCLK_KHZ)     /* 100ms */

struct scpi_mem {
	struct scpi_msg tx_msg; /**< The reply to be sent to a client. */
	struct scpi_msg rx_msg; /**< The request received from a client. */
};

struct scpi_buffer {
	struct scpi_mem mem;    /**< Memory for the request/reply messages. */
	uint8_t         client; /**< Client that should receive the reply. */
	bool            busy;   /**< Flag telling if this buffer is in use. */
};

/** The message box device used for communication with SCPI clients. */
static struct device *scpi_msgbox;

/** Buffers used to hold messages while they are being processed. */
static struct scpi_buffer scpi_buffers[SCPI_BUFFER_MAX];

/**
 * Allocate a temporary buffer for processing an SCPI message.
 *
 * Because this function is called from both IRQ handlers and process context,
 * it must be locked.
 */
static struct scpi_buffer *
scpi_alloc_buffer(uint8_t client)
{
	struct scpi_buffer *buffer = NULL;
	uint32_t flags;

	flags = disable_interrupts();
	for (size_t i = 0; i < SCPI_BUFFER_MAX; ++i) {
		if (!scpi_buffers[i].busy) {
			buffer         = &scpi_buffers[i];
			buffer->busy   = true;
			buffer->client = client;
			break;
		}
	}
	restore_interrupts(flags);

	return buffer;
}

/**
 * Free a temporary buffer after processing an SCPI message.
 */
static inline void
scpi_free_buffer(struct scpi_buffer *buffer)
{
	/* Prevent the compiler from reordering the store (the unlock
	 * operation) before earlier operations. */
	barrier();
	buffer->busy = false;
}

/**
 * Copy an SCPI message to or from shared memory. This function examines the
 * message's payload size field and only copies as many bytes as are necessary.
 * To work around hardware byte swapping, it copies four bytes at a time.
 */
static void
scpi_copy_message(struct scpi_msg *dest, struct scpi_msg *src)
{
	uint32_t *d = (uint32_t *)dest;
	uint32_t *s = (uint32_t *)src;

	for (int n = src->size + SCPI_HEADER_SIZE; n > 0; n -= 4)
		*d++ = *s++;
}

/**
 * Part 3 of SCPI message handling.
 *
 * Send a prepared message once the shared memory area is free (the AP has
 * acknowledged our last message). Try hard to avoid dropping messages. If
 * waiting for an acknowledgement times out, do other work and try again later.
 */
static void
scpi_send_message(void *param)
{
	int err;
	struct scpi_buffer *buffer = param;
	uint8_t  client  = buffer->client;
	uint64_t timeout = wallclock_read() + SCPI_TX_TIMEOUT;

	/* Wait for any previous message to be acknowledged, with a timeout. */
	while (msgbox_tx_pending(scpi_msgbox, buffer->client)) {
		if (wallclock_read() >= timeout) {
			warn("SCPI.%u: Channel busy", client);
			scpi_free_buffer(buffer);
			return;
		}
	}

	/* Copy the prepared reply from the buffer to shared memory. */
	scpi_copy_message(&SCPI_MEM_AREA(client).tx_msg, &buffer->mem.tx_msg);

	/* Notify the client that the message has been sent. */
	if ((err = msgbox_send(scpi_msgbox, client, SCPI_VIRTUAL_CHANNEL)))
		error("SCPI.%u: Send error: %d", client, err);

	/* Mark the buffer as no longer in use. */
	scpi_free_buffer(buffer);
}

/**
 * Parts 1 & 2 of SCPI message handling (SCP-initiated messages).
 */
int
scpi_create_message(uint8_t client, uint8_t command, uint32_t *payload,
                    uint16_t payload_size)
{
	struct scpi_buffer *buffer = scpi_alloc_buffer(client);

	/* Ensure there is space to put the created message. */
	if (buffer == NULL)
		return EBUSY;

	/* Create the message header. */
	buffer->mem.tx_msg.command = command;
	buffer->mem.tx_msg.sender  = SCPI_SENDER_SCP;
	buffer->mem.tx_msg.size    = payload_size;
	buffer->mem.tx_msg.status  = SCPI_OK;

	/* Copy the payload to the buffer, if there is one. */
	if (payload_size > 0)
		memcpy(&buffer->mem.tx_msg, payload, payload_size);

	queue_work(scpi_send_message, buffer);

	return SUCCESS;
}

/**
 * Part 2 of SCPI message handling.
 *
 * Dispatch handling of the message to the appropriate function for each
 * command. The functions for doing so are defined in a separate file to
 * separate the API functionality from communication/synchronization code.
 *
 * When the command's actions are complete, and a reply message is created,
 * enqueue a function to send the response.
 */
static void
scpi_handle_message(void *param)
{
	struct scpi_buffer *buffer = param;

	assert(buffer->busy);

	/* Create a reply in tx_msg based on the contents of rx_msg. */
	if (!scpi_handle_cmd(buffer->client, &buffer->mem.rx_msg,
	                     &buffer->mem.tx_msg)) {
		/* If the message does not need a reply, stop processing. */
		scpi_free_buffer(buffer);
		return;
	}

	queue_work(scpi_send_message, buffer);
}

/**
 * Part 1 of SCPI message handling (received messages).
 *
 * This is the callback from the message box framework; it is called when a new
 * message is received. The SCPI client regains ownership of the shared memory
 * area once the message is acknowledged, which happens when the message box
 * driver clears the pending IRQ, immediately after this function returns.
 * Thus, we must do our best to copy the shared memory area's contents to a
 * safe before returning.
 *
 * Once the message is in a location controlled by the SCP firmware, enqueue a
 * work item to parse, handle, and respond to it. This allows the interrupt to
 * finish as quickly as possible.
 */
static void
scpi_receive_message(struct device *dev __unused, uint8_t client, uint32_t msg)
{
	struct scpi_buffer *buffer;
	struct scpi_msg    *rx_msg = &SCPI_MEM_AREA(client).rx_msg;

	assert(client == SCPI_CLIENT_NS || client == SCPI_CLIENT_SECURE);

	/* Do not try to parse messages sent with a different protocol. */
	if (msg != SCPI_VIRTUAL_CHANNEL)
		return;

	/* Ensure there is a place to put the received message. */
	buffer = scpi_alloc_buffer(client);
	if (buffer == NULL) {
		error("SCPI.%u: No free buffer", client);
		return;
	}

	/* Perform minimal validation of the message to optimize copying it.
	 * No valid message will be too large (because conforming clients will
	 * honor our advertised maximum payload size), so this guards against a
	 * malicious client or reserved bits being set. */
	if (rx_msg->size > SCPI_PAYLOAD_SIZE)
		rx_msg->size = SCPI_PAYLOAD_SIZE;
	/* Save the received message outside the shared memory area. */
	scpi_copy_message(&buffer->mem.rx_msg, rx_msg);

	queue_work(scpi_handle_message, buffer);
}

void
scpi_init(void)
{
	int err;

	if ((scpi_msgbox = dm_first_dev_by_class(DM_CLASS_MSGBOX)) == NULL)
		panic("SCPI: No message box device");
	/* Non-secure client channel. */
	if ((err = msgbox_enable(scpi_msgbox, SCPI_CLIENT_NS,
	                         scpi_receive_message)))
		panic("SCPI.%u: Error registering handler: %d",
		      SCPI_CLIENT_NS, err);
	/* Secure client channel. */
	if ((err = msgbox_enable(scpi_msgbox, SCPI_CLIENT_SECURE,
	                         scpi_receive_message)))
		panic("SCPI.%u: Error registering handler: %d",
		      SCPI_CLIENT_SECURE, err);

	scpi_create_message(SCPI_CLIENT_SECURE, SCPI_CMD_SCP_READY, NULL, 0);

	info("SCPI: Initialization complete");
}
