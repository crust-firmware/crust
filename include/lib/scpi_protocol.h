/*
 * Copyright © 2014-2017, ARM Limited and Contributors. All rights reserved.
 * Copyright © 2017-2018 The Crust Firmware Authors.
 * SPDX-License-Identifier: BSD-3-Clause OR GPL-2.0-only
 */

#ifndef COMMON_SCPI_PROTOCOL_H
#define COMMON_SCPI_PROTOCOL_H

#include <compiler.h>
#include <stdint.h>

/** The SCPI message header is specified to be 64 bits long. */
#define SCPI_HEADER_SIZE     sizeof(uint64_t)

/** The implementation-defined maximum size of an SCPI message. */
#define SCPI_MESSAGE_SIZE    0x100

/** The payload can use all but the 64 bits reserved for the header. */
#define SCPI_PAYLOAD_SIZE    (SCPI_MESSAGE_SIZE - SCPI_HEADER_SIZE)

/** The payload is represented as an array of 32-bit words. */
#define SCPI_PAYLOAD_WORDS   (SCPI_PAYLOAD_SIZE / sizeof(uint32_t))

/** The SCP must identify itself as sender 0 for messages it initiates. */
#define SCPI_SENDER_SCP      0

/**
 * The virtual channel number is defined by the SCPI specification.
 * Theoretically, this allows reusing the same shared memory area for different
 * types of messages, if a different virtual channel number is given. In this
 * implementation, the virtual channel number is the contents of the mailbox
 * message. Messages with any other virtual channel are ignored.
 */
#define SCPI_VIRTUAL_CHANNEL BIT(0)

/**
 * The set of standard SCPI commands, defined by the SCPI specification.
 */
enum {
	SCPI_CMD_SCP_READY         = 0x01, /**< SCP ready. */
	SCPI_CMD_GET_SCP_CAP       = 0x02, /**< Get SCP capability. */
	SCPI_CMD_SET_CSS_PWR       = 0x03, /**< Set CSS power state. */
	SCPI_CMD_GET_CSS_PWR       = 0x04, /**< Get CSS power state. */
	SCPI_CMD_SET_SYS_PWR       = 0x05, /**< Set system power state. */
	SCPI_CMD_SET_CPU_TIMER     = 0x06, /**< Set CPU timer. */
	SCPI_CMD_CANCEL_CPU_TIMER  = 0x07, /**< Cancel CPU timer. */
	SCPI_CMD_GET_DVFS_CAP      = 0x08, /**< Get DVFS capability. */
	SCPI_CMD_GET_DVFS_INFO     = 0x09, /**< Get DVFS info. */
	SCPI_CMD_SET_DVFS          = 0x0a, /**< Set DVFS. */
	SCPI_CMD_GET_DVFS          = 0x0b, /**< Get DVFS. */
	SCPI_CMD_GET_DVFS_STATS    = 0x0c, /**< Get DVFS statistics. */
	SCPI_CMD_GET_CLOCK_CAP     = 0x0d, /**< Get clock capability. */
	SCPI_CMD_GET_CLOCK_INFO    = 0x0e, /**< Get clock info. */
	SCPI_CMD_SET_CLOCK         = 0x0f, /**< Set clock value. */
	SCPI_CMD_GET_CLOCK         = 0x10, /**< Get clock value. */
	SCPI_CMD_GET_PSU_CAP       = 0x11, /**< Get power supply capability. */
	SCPI_CMD_GET_PSU_INFO      = 0x12, /**< Get power supply info. */
	SCPI_CMD_SET_PSU           = 0x13, /**< Set power supply. */
	SCPI_CMD_GET_PSU           = 0x14, /**< Get power supply. */
	SCPI_CMD_GET_SENSOR_CAP    = 0x15, /**< Get sensor capability. */
	SCPI_CMD_GET_SENSOR_INFO   = 0x16, /**< Get sensor info. */
	SCPI_CMD_GET_SENSOR        = 0x17, /**< Get sensor value. */
	SCPI_CMD_CFG_SENSOR_PERIOD = 0x18, /**< Configure sensor period. */
	SCPI_CMD_CFG_SENSOR_BOUNDS = 0x19, /**< Configure sensor bounds. */
	SCPI_CMD_ASYNC_SENSOR      = 0x1a, /**< Asynchronous sensor value. */
	SCPI_CMD_SET_DEV_PWR       = 0x1b, /**< Set device power state. */
	SCPI_CMD_GET_DEV_PWR       = 0x1c, /**< Get device power state. */
};

/**
 * The set of possible status codes in an SCPI message, defined by the SCPI
 * specification.
 */
enum {
	SCPI_OK         = 0,  /**< Success. */
	SCPI_E_PARAM    = 1,  /**< Invalid parameter(s). */
	SCPI_E_ALIGN    = 2,  /**< Invalid alignment. */
	SCPI_E_SIZE     = 3,  /**< Invalid size. */
	SCPI_E_HANDLER  = 4,  /**< Invalid handler or callback. */
	SCPI_E_ACCESS   = 5,  /**< Invalid access or permission denied. */
	SCPI_E_RANGE    = 6,  /**< Value out of range. */
	SCPI_E_TIMEOUT  = 7,  /**< Timeout has occurred. */
	SCPI_E_NOMEM    = 8,  /**< Invalid memory area or pointer. */
	SCPI_E_PWRSTATE = 9,  /**< Invalid power state. */
	SCPI_E_SUPPORT  = 10, /**< Feature not supported or disabled. */
	SCPI_E_DEVICE   = 11, /**< Device error. */
	SCPI_E_BUSY     = 12, /**< Device is busy. */
	SCPI_E_OS       = 13, /**< RTOS error occurred. */
	SCPI_E_DATA     = 14, /**< Unexpected or invalid data received. */
	SCPI_E_STATE    = 15, /**< Invalid or unattainable state requested. */
};

/**
 * The memory structure representing an SCPI message, defined by the SCPI
 * specification.
 *
 * The structure below does not exactly follow the specification. The set ID
 * bit has been merged into the command number, because it is semantically
 * meaningless. And the reserved bits have been merged into the payload size.
 * Any command with reserved bits set will be interpreted as being "too large"
 * and will be rejected.
 *
 * The fields in the first 32-bit word are reversed from their order in the
 * specification to account for hardware byte swapping. The payload is
 * represented as an array of 32-bit words to reduce the amount of byte
 * swapping needed in command implementations.
 */
struct scpi_msg {
#ifdef __or1k__
	uint16_t size;
	uint8_t  sender;
	uint8_t  command;
#else
	uint8_t  command;
	uint8_t  sender;
	uint16_t size;
#endif
	uint32_t status;
#ifdef __or1k__
	uint32_t payload[SCPI_PAYLOAD_WORDS];
#else
	uint8_t  payload[SCPI_PAYLOAD_SIZE];
#endif
};

#endif /* COMMON_SCPI_PROTOCOL_H */
