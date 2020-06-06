/*
 * Copyright © 2017-2020 The Crust Firmware Authors.
 * SPDX-License-Identifier: BSD-3-Clause OR GPL-2.0-only
 */

#include <assert.h>
#include <fcntl.h>
#include <limits.h>
#include <setjmp.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdnoreturn.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <sys/mman.h>

#include <compiler.h>
#include <config.h>
#include <kconfig.h>
#include <mmio.h>
#include <scpi_protocol.h>
#include <util.h>
#include <platform/devices.h>
#include <platform/memory.h>

/* SCPI device (clock/power supply) flags. */
#define FLAG_READABLE               BIT(0)
#define FLAG_WRITABLE               BIT(1)

/* Simplified version of the message box register definitions. This tool only
 * uses virtual channel 0 (hardware channels 0/1). */
#define MSGBOX_LOCAL_IRQ_STATUS_REG 0x0070
#define MSGBOX_LOCAL_RX_IRQ         BIT(2)

#define MSGBOX_ARISC_IRQ_STATUS_REG 0x0050
#define MSGBOX_ARISC_RX_IRQ         BIT(0)

#define MSGBOX_RX_MSG_STATUS_REG    0x0144
#define MSGBOX_TX_MSG_STATUS_REG    0x0140
#define MSGBOX_MSG_STATUS_MASK      GENMASK(2, 0)

#define MSGBOX_RX_MSG_DATA_REG      0x0184
#define MSGBOX_TX_MSG_DATA_REG      0x0180

#define SENSOR_CLASS_TEMPERATURE    0
#define SENSOR_CLASS_COUNT          5

/* PAGESIZE is not defined on architectures with variable-sized pages. Assume
 * 4k pages if the size is unknown. */
#ifndef PAGESIZE
#define PAGESIZE          0x1000
#endif
#define PAGE_BASE(addr)   ((addr) & ~(PAGESIZE - 1))
#define PAGE_OFFSET(addr) ((addr) & (PAGESIZE - 1))

/** Shorthand for the virtual mmapped address of the shared memory area. */
#define SCPI_SHMEM        (&((struct scpi_mem *)sram)[1])

/** Arbitrary value to identify messages sent by the test program. */
#define SCPI_SENDER_TEST  0xaa

/** Arbitrary value to ensure the firmware correctly handles status codes. */
#define SCPI_STATUS_TEST  0xcafef00d

/** Convert a symbol to a string. */
#define STRINGIFY(token)  #token

/** Fail the current test if some condition is false. */
#define test_assert(x)    ((void)((x) || (test_assert_fail(#x, __LINE__), 0)))

/** Begin a test. */
#define test_begin(id) \
	do { \
		tests_attempted |= BIT(id); \
		if (sigsetjmp(test_buf, 0)) { \
			test_fail(id); \
			return; \
		} \
	} while (0)

/** Severities for log levels. */
enum {
	LOG_INFO,  /**< An informational message. */
	LOG_WARN,  /**< A warning. The current test will continue. */
	LOG_ERR,   /**< An error. The current test cannot continue. */
	LOG_FATAL, /**< A fatal error. Testing the firmware cannot continue. */
};

/** Identifiers for the various tests. */
enum {
	TEST_BOOT,
	TEST_INTERRUPT,
	TEST_SCP_CAP,
	TEST_CLOCK_CAP,
	TEST_CLOCK_CMDS,
	TEST_CLOCK_INFO,
	TEST_CLOCK_CTRL,
	TEST_CSS_INFO,
	TEST_DVFS_CAP,
	TEST_DVFS_CMDS,
	TEST_DVFS_INFO,
	TEST_DVFS_CTRL,
	TEST_PSU_CAP,
	TEST_PSU_CMDS,
	TEST_PSU_INFO,
	TEST_PSU_CTRL,
	TEST_SENSOR_CAP,
	TEST_SENSOR_CMDS,
	TEST_SENSOR_INFO,
	TEST_SENSOR_READ,
	TEST_SYS_POWER,
	TEST_COUNT,
};

/** A structure containing the timing data needed to analyze an SCPI call. */
struct scpi_call_times {
	struct timespec start;       /**< Before doing any processing. */
	struct timespec send;        /**< Before the call to msgbox_send. */
	struct timespec send_ack;    /**< After return from msgbox_send. */
	struct timespec receive;     /**< After return from msgbox_receive. */
	struct timespec receive_ack; /**< After return from msgbox_ack. */
	struct timespec finish;      /**< After all processing is complete. */
};

/** Context for returning to the main program on failure. */
static sigjmp_buf main_buf;
/** Context for continuing with the next test on failure. */
static sigjmp_buf test_buf;

/** Prefixes for the various log levels. */
static const char *const log_prefixes[] = {
	"",
	"warning: ",
	"error: ",
	"fatal: "
};

/** Virtual address of the mmapped msgbox registers. */
static uintptr_t mbox;
/** Virtual address of the mmapped SCPI shared memory in SRAM A2. */
static uintptr_t sram;

/** SCPI command names printed with timing information. */
static const char *const scpi_command_names[] = {
	"INVALID SCPI COMMAND",
	STRINGIFY(SCPI_CMD_SCP_READY),
	STRINGIFY(SCPI_CMD_GET_SCP_CAP),
	STRINGIFY(SCPI_CMD_SET_CSS_PWR),
	STRINGIFY(SCPI_CMD_GET_CSS_PWR),
	STRINGIFY(SCPI_CMD_SET_SYS_PWR),
	STRINGIFY(SCPI_CMD_SET_CPU_TIMER),
	STRINGIFY(SCPI_CMD_CANCEL_CPU_TIMER),
	STRINGIFY(SCPI_CMD_GET_DVFS_CAP),
	STRINGIFY(SCPI_CMD_GET_DVFS_INFO),
	STRINGIFY(SCPI_CMD_SET_DVFS),
	STRINGIFY(SCPI_CMD_GET_DVFS),
	STRINGIFY(SCPI_CMD_GET_DVFS_STATS),
	STRINGIFY(SCPI_CMD_GET_CLOCK_CAP),
	STRINGIFY(SCPI_CMD_GET_CLOCK_INFO),
	STRINGIFY(SCPI_CMD_SET_CLOCK),
	STRINGIFY(SCPI_CMD_GET_CLOCK),
	STRINGIFY(SCPI_CMD_GET_PSU_CAP),
	STRINGIFY(SCPI_CMD_GET_PSU_INFO),
	STRINGIFY(SCPI_CMD_SET_PSU),
	STRINGIFY(SCPI_CMD_GET_PSU),
	STRINGIFY(SCPI_CMD_GET_SENSOR_CAP),
	STRINGIFY(SCPI_CMD_GET_SENSOR_INFO),
	STRINGIFY(SCPI_CMD_GET_SENSOR),
	STRINGIFY(SCPI_CMD_CFG_SENSOR_PERIOD),
	STRINGIFY(SCPI_CMD_CFG_SENSOR_BOUNDS),
	STRINGIFY(SCPI_CMD_ASYNC_SENSOR),
	STRINGIFY(SCPI_CMD_SET_DEV_PWR),
	STRINGIFY(SCPI_CMD_GET_DEV_PWR),
};

/** A bitmap of the available SCPI commands, from SCPI_CMD_GET_SCP_CAP. */
static uint32_t scpi_commands_available;

/** Human-readable names for the various tests. */
static const char *const test_names[TEST_COUNT] = {
	"Boot",
	"Interrupt",
	"SCP capability",
	"Clock capability",
	"Clock commands",
	"Clock info",
	"Clock control",
	"CSS info",
	"DVFS capability",
	"DVFS commands",
	"DVFS info",
	"DVFS control",
	"PSU capability",
	"PSU commands",
	"PSU info",
	"PSU control",
	"Sensor capability",
	"Sensor commands",
	"Sensor info",
	"Sensor read",
	"System power",
};

/** A bitmap of attempted tests. */
static unsigned long tests_attempted;
/** A bitmap of failed tests. */
static unsigned long tests_failed;
/** A bitmap of passed tests. */
static unsigned long tests_passed;

/**
 * Get the number of set bits in a bitmap.
 */
static unsigned
bitmap_weight(unsigned long bits)
{
	return __builtin_popcount(bits);
}

/**
 * Clean and invalidate lines in the data cache associated with a range of
 * virtual addresses.
 */
static void
data_cache_clean(void *start, size_t length)
{
#ifdef __aarch64__
	uintptr_t addr = (uintptr_t)start;
	uintptr_t end = (uintptr_t)start + length;
	uintptr_t order, stride;

	/* Ensure the compiler doesn't move stores past the cache cleaning. */
	barrier();
	/* Extract the order of the size of the smallest cache line. */
	asm ("mrs  %0, CTR_EL0\n\t"
             "ubfx %0, %0, #16, #4" : "=r" (order));
	/* Get the actual stride length from its order. */
	stride = BIT(order);
	/* Align the address to the stride, and clean the cache by VA. */
	for (addr &= ~(stride - 1); addr < end; addr += stride)
		asm volatile ("dc civac, %0" : : "r" (addr));
	asm volatile ("dsb sy");
#else
#error "No cache cleaning implementation available for this architecture"
#endif
}

/**
 * Get the difference between two timespecs in nanoseconds.
 */
static long
difftimespec(const struct timespec *x, const struct timespec *y)
{
	return 1000000000L * (x->tv_sec - y->tv_sec) + x->tv_nsec - y->tv_nsec;
}

/**
 * Log a message to standard error, and possibly abort testing.
 */
static void
log(unsigned level, const char *fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);
	fflush(NULL);
	fputs(log_prefixes[level], stderr);
	vfprintf(stderr, fmt, ap);
	fputc('\n', stderr);
	va_end(ap);

	if (level >= LOG_FATAL) {
		/* Fatal error: print summary and exit. */
		siglongjmp(main_buf, 1);
	} else if (level >= LOG_ERR) {
		/* Non-fatal error: fail the test and resume with the next. */
		siglongjmp(test_buf, 1);
	}
}

/**
 * Acknowledge that message has been received and its associated shared memory
 * is again available for use.
 */
static void
msgbox_ack(void)
{
	/* Clear the local message reception IRQ. */
	mmio_write_32(mbox + MSGBOX_LOCAL_IRQ_STATUS_REG, MSGBOX_LOCAL_RX_IRQ);
}

/**
 * Wait for a message, and ensure that it is an SCPI message.
 */
static void
msgbox_receive(void)
{
	uint32_t msgs, reg;

	/* Wait for a message reception IRQ. */
	mmio_poll_32(mbox + MSGBOX_LOCAL_IRQ_STATUS_REG, MSGBOX_LOCAL_RX_IRQ);
	/* Read the number of available messages. */
	reg = mmio_read_32(mbox + MSGBOX_RX_MSG_STATUS_REG);
	if (unlikely((msgs = reg & MSGBOX_MSG_STATUS_MASK) > 1))
		log(LOG_WARN, "Expected one message, received %u", msgs);
	else if (unlikely(msgs == 0))
		log(LOG_FATAL, "Received an IRQ, but no messages");
	reg = mmio_read_32(mbox + MSGBOX_RX_MSG_DATA_REG);
	if (unlikely(reg != SCPI_VIRTUAL_CHANNEL))
		log(LOG_FATAL, "Expected SCPI, received protocol 0x%08x", reg);
}

/**
 * Send a message, and wait for it to be acknowledged.
 */
static void
msgbox_send(void)
{
	/* Send the message. */
	mmio_write_32(mbox + MSGBOX_TX_MSG_DATA_REG, SCPI_VIRTUAL_CHANNEL);
	/* Wait for the firmware to acknowledge the message. */
	mmio_pollz_32(mbox + MSGBOX_ARISC_IRQ_STATUS_REG,
	              MSGBOX_ARISC_RX_IRQ);
}

/**
 * Get the actual size of an SCPI message in bytes.
 */
static size_t
scpi_msg_size(struct scpi_msg *msg)
{
	return SCPI_HEADER_SIZE + msg->size;
}

/**
 * Copy an SCPI message, ensuring cache coherency.
 */
static void
scpi_copy_msg(struct scpi_msg *dest, struct scpi_msg *src)
{
	size_t length = scpi_msg_size(src);

	/* Only copy the necessary number of bytes from the message. */
	memcpy(dest, src, length);

	/* Ensure that there is no stale cache of the shared memory area. */
	if (dest == &SCPI_SHMEM->rx_msg)
		data_cache_clean(dest, length);
	else if (src == &SCPI_SHMEM->tx_msg)
		data_cache_clean(src, length);
}

/**
 * Check if a command is in the set of available SCPI commands.
 */
static bool
scpi_has_command(uint8_t id)
{
	if (id >= CHAR_BIT * sizeof(scpi_commands_available))
		return false;

	return (scpi_commands_available & BIT(id)) == BIT(id);
}

/**
 * Initialize the header of an SCPI message.
 */
static void
scpi_prepare_msg(struct scpi_msg *msg, uint8_t command)
{
	msg->command = command;
	msg->sender  = SCPI_SENDER_TEST;
	msg->size    = 0;
	msg->status  = SCPI_STATUS_TEST;
}

/**
 * Send an SCPI request and verify its success.
 */
static bool
scpi_send_request(struct scpi_msg *msg, struct scpi_call_times *times)
{
	/* Save the original command and sender to compare with the reply. */
	uint8_t  sender  = msg->sender;
	uint32_t command = msg->command;

	clock_gettime(CLOCK_MONOTONIC, &times->start);
	/* Copy the message to shared memory. */
	scpi_copy_msg(&SCPI_SHMEM->rx_msg, msg);
	/* Send the message. */
	clock_gettime(CLOCK_MONOTONIC, &times->send);
	msgbox_send();
	clock_gettime(CLOCK_MONOTONIC, &times->send_ack);
	/* Wait for a response message. */
	msgbox_receive();
	clock_gettime(CLOCK_MONOTONIC, &times->receive);
	/* Copy the message from shared memory to DRAM. */
	scpi_copy_msg(msg, &SCPI_SHMEM->tx_msg);
	/* Acknowledge the response, now that it's safely stored in DRAM. */
	msgbox_ack();
	clock_gettime(CLOCK_MONOTONIC, &times->receive_ack);
	/* Stop the timer. */
	clock_gettime(CLOCK_MONOTONIC, &times->finish);

	/* Verify that the received message is an appropriate response. */
	if (unlikely(msg->command != command)) {
		log(LOG_ERR, "Expected reply for command %u, received %u",
		    command, msg->command);
		return false;
	}
	if (unlikely(msg->sender != sender)) {
		log(LOG_ERR, "Expected reply to sender %u, received %u",
		    sender, msg->sender);
		return false;
	}

	return true;
}

/**
 * Wait for an SCPI request and send a reply.
 */
static bool
scpi_serve_reply(struct scpi_msg *msg, struct scpi_call_times *times,
                 uint32_t command)
{
	clock_gettime(CLOCK_MONOTONIC, &times->start);
	/* Wait for a message to arrive. */
	msgbox_receive();
	clock_gettime(CLOCK_MONOTONIC, &times->receive);
	/* Copy the message from shared memory to DRAM. */
	scpi_copy_msg(msg, &SCPI_SHMEM->tx_msg);
	/* Acknowledge that the message was received. */
	msgbox_ack();
	clock_gettime(CLOCK_MONOTONIC, &times->receive_ack);
	/* Prepare the reply message (a copy of the received message). */
	if (likely(msg->command == command && msg->sender == SCPI_SENDER_SCP))
		msg->status = SCPI_OK;
	else
		msg->status = SCPI_E_PARAM;
	scpi_copy_msg(&SCPI_SHMEM->rx_msg, msg);
	/* Send the reply message. */
	clock_gettime(CLOCK_MONOTONIC, &times->send);
	msgbox_send();
	clock_gettime(CLOCK_MONOTONIC, &times->send_ack);

	/* Verify that the received message is as expected. */
	if (unlikely(msg->command != command)) {
		log(LOG_ERR, "Expected command %u, received %u",
		    command, msg->command);
		return false;
	}
	if (unlikely(msg->sender != SCPI_SENDER_SCP)) {
		log(LOG_ERR, "Expected sender %u, got sender %u",
		    SCPI_SENDER_SCP, msg->sender);
		return false;
	}

	return true;
}

/**
 * Handle failure of a test.
 */
static noreturn void
test_assert_fail(const char *expr, unsigned line)
{
	log(LOG_ERR, "Assertion failed: %d: '%s'", line, expr);

	siglongjmp(test_buf, -1);
}

/**
 * Mark a test as completed (passed).
 */
static void
test_complete(uint8_t id)
{
	if (tests_passed & BIT(id))
		return;

	tests_passed |= BIT(id);
	printf("PASS: %s\n", test_names[id]);
}

/**
 * Mark a test as failed.
 */
static void
test_fail(uint8_t id)
{
	if (tests_failed & BIT(id))
		return;

	tests_failed |= BIT(id);
	printf("FAIL: %s\n", test_names[id]);
}

/**
 * Send an SCPI request as part of a test.
 */
static void
test_send_request(struct scpi_msg *msg)
{
	struct scpi_call_times times;

	test_assert(scpi_send_request(msg, &times));
	printf("TIME: %-25s │ send: %10ldns │ receive: %10ldns │ busy: %10ldns"
	       " │ total: %10ldns\n",
	       scpi_command_names[msg->command],
	       difftimespec(&times.send_ack, &times.send),
	       difftimespec(&times.receive_ack, &times.receive),
	       difftimespec(&times.receive_ack, &times.send),
	       difftimespec(&times.finish, &times.start));
}

/**
 * Wait for an SCPI request as part of a test.
 */
static void
test_serve_reply(struct scpi_msg *msg, long *wait_time)
{
	struct scpi_call_times times;

	test_assert(scpi_serve_reply(msg, &times, msg->command));
	*wait_time = difftimespec(&times.receive, &times.start);
}

/**
 * Print a summary of tests run.
 */
static void
test_summary(void)
{
	printf("DONE: %u tests, %u passed, %u failed, %u skipped\n",
	       TEST_COUNT,
	       bitmap_weight(tests_passed),
	       bitmap_weight(tests_failed),
	       TEST_COUNT - bitmap_weight(tests_attempted));
}

/*
 * Test: Basic commands (SCP capability).
 */
static void
try_basic(void)
{
	struct scpi_msg msg;

	/* Send an invalid command to test basic message processing. */
	test_begin(TEST_INTERRUPT);
	scpi_prepare_msg(&msg, 0);
	test_send_request(&msg);
	test_assert(msg.status == SCPI_E_SUPPORT);
	test_assert(msg.size == 0);

	/* Send a valid command with the wrong payload size. */
	scpi_prepare_msg(&msg, SCPI_CMD_GET_SCP_CAP);
	msg.size = 4;
	test_send_request(&msg);
	test_assert(msg.status == SCPI_E_SIZE);
	test_assert(msg.size == 0);
	test_complete(TEST_INTERRUPT);

	/* Query the firmware's capabilities. Save the resulting set of
	 * supported commands in a global variable for later use. */
	test_begin(TEST_SCP_CAP);
	scpi_prepare_msg(&msg, SCPI_CMD_GET_SCP_CAP);
	test_send_request(&msg);
	test_assert(msg.status == SCPI_OK);
	test_assert(msg.size == 28);
	/* The supported SCPI standard must be revision 1.2. */
	test_assert(((uint16_t *)msg.payload)[0] == 2);
	test_assert(((uint16_t *)msg.payload)[1] == 1);
	/* The supported payload size must match the header definitions. */
	test_assert(((uint16_t *)msg.payload)[2] == SCPI_PAYLOAD_SIZE);
	test_assert(((uint16_t *)msg.payload)[3] == SCPI_PAYLOAD_SIZE);
	scpi_commands_available = ((uint32_t *)msg.payload)[3];
	/* Ensure the supported commands word is sane (that the firmware claims
	 * to support the commands we just sent). */
	test_assert(scpi_has_command(SCPI_CMD_SCP_READY));
	test_assert(scpi_has_command(SCPI_CMD_GET_SCP_CAP));
	test_complete(TEST_SCP_CAP);
}

/*
 * Test: Boot (SCP ready).
 */
static void
try_boot(void)
{
	struct scpi_msg msg;
	long boot_time;

	/* Seeing "SCP ready" means the firmware booted successfully. */
	log(LOG_INFO, "Waiting for firmware...");
	test_begin(TEST_BOOT);
	scpi_prepare_msg(&msg, SCPI_CMD_SCP_READY);
	test_serve_reply(&msg, &boot_time);
	log(LOG_INFO, "Firmware booted in %ld ns", boot_time);
	test_complete(TEST_BOOT);
}

/*
 * Test: Clocks.
 */
static void
try_clocks(void)
{
	struct scpi_msg msg;
	uint16_t clocks = 0;

	/* Skip this test if the required commands are not available. */
	if (!scpi_has_command(SCPI_CMD_GET_CLOCK_CAP))
		return;

	/* Get the number of clocks. */
	test_begin(TEST_CLOCK_CAP);
	scpi_prepare_msg(&msg, SCPI_CMD_GET_CLOCK_CAP);
	test_send_request(&msg);
	test_assert(msg.status == SCPI_OK);
	test_assert(msg.size == 2);
	clocks = ((uint16_t *)msg.payload)[0];
	test_complete(TEST_CLOCK_CAP);

	/* If the firmware has clocks, it must fully support clock control. */
	if (clocks > 0) {
		test_begin(TEST_CLOCK_CMDS);
		test_assert(scpi_has_command(SCPI_CMD_GET_CLOCK_INFO));
		test_assert(scpi_has_command(SCPI_CMD_SET_CLOCK));
		test_assert(scpi_has_command(SCPI_CMD_GET_CLOCK));
		test_complete(TEST_CLOCK_CMDS);
	}

	for (volatile uint16_t i = 0; i < clocks; ++i) {
		uint8_t  flags;
		uint32_t max_rate, min_rate;

		test_begin(TEST_CLOCK_INFO);
		scpi_prepare_msg(&msg, SCPI_CMD_GET_CLOCK_INFO);
		/* Send the clock ID. */
		msg.size = 2;
		((uint16_t *)msg.payload)[0] = i;
		test_send_request(&msg);
		test_assert(msg.status == SCPI_OK);
		test_assert(msg.size >= 12);
		/* Assert that we got the same ID back. */
		test_assert(((uint16_t *)msg.payload)[0] == i);
		/* Assert that there are no reserved flags. */
		flags = ((uint16_t *)msg.payload)[1];
		test_assert((flags & ~(FLAG_READABLE | FLAG_WRITABLE)) == 0);
		/* Assert that the minimum and maximum rates make sense. */
		min_rate = ((uint32_t *)msg.payload)[1];
		max_rate = ((uint32_t *)msg.payload)[2];
		test_assert(max_rate >= min_rate);
		test_complete(TEST_CLOCK_INFO);

		/* Try setting both the minimum and maximum rates. */
		test_begin(TEST_CLOCK_CTRL);
		scpi_prepare_msg(&msg, SCPI_CMD_SET_CLOCK);
		msg.size = 8;
		((uint32_t *)msg.payload)[0] = i;
		((uint32_t *)msg.payload)[1] = min_rate;
		test_send_request(&msg);
		/* Assert that the firmware respects its permission flags. */
		if (!(flags & FLAG_WRITABLE)) {
			test_assert(msg.status == SCPI_E_ACCESS);
		} else {
			/* Set the clock may still fail if it is in use. */
			test_assert(msg.status == SCPI_OK ||
			            msg.status == SCPI_E_ACCESS);
		}
		test_assert(msg.size == 0);

		scpi_prepare_msg(&msg, SCPI_CMD_SET_CLOCK);
		msg.size = 8;
		((uint32_t *)msg.payload)[0] = i;
		((uint32_t *)msg.payload)[1] = max_rate;
		test_send_request(&msg);
		/* Assert that the firmware respects its permission flags. */
		if (!(flags & FLAG_WRITABLE)) {
			test_assert(msg.status == SCPI_E_ACCESS);
		} else {
			/* Set the clock may still fail if it is in use. */
			test_assert(msg.status == SCPI_OK ||
			            msg.status == SCPI_E_ACCESS);
		}
		test_assert(msg.size == 0);

		/* Read back clock rate and ensure it is in range. */
		scpi_prepare_msg(&msg, SCPI_CMD_GET_CLOCK);
		msg.size = 2;
		((uint16_t *)msg.payload)[0] = i;
		test_send_request(&msg);
		/* Assert that the firmware respects its permission flags. */
		if (!(flags & FLAG_READABLE)) {
			test_assert(msg.status == SCPI_E_ACCESS);
			test_complete(TEST_CLOCK_CTRL);
			continue;
		}
		test_assert(msg.status == SCPI_OK);
		test_assert(msg.size == 4);
		test_assert(((uint32_t *)msg.payload)[0] >= min_rate);
		test_assert(((uint32_t *)msg.payload)[0] <= max_rate);
		test_complete(TEST_CLOCK_CTRL);
	}

	/* Test the failure case of sending an invalid clock ID. */
	test_begin(TEST_CLOCK_INFO);
	scpi_prepare_msg(&msg, SCPI_CMD_GET_CLOCK_INFO);
	msg.size = 2;
	((uint16_t *)msg.payload)[0] = clocks;
	test_send_request(&msg);
	test_assert(msg.status == SCPI_E_PARAM);
	test_complete(TEST_CLOCK_INFO);
}

/*
 * Test: CSS power.
 */
static void
try_css_power(void)
{
	struct scpi_msg msg;

	/* Skip this test if the required commands are not available. */
	if (!scpi_has_command(SCPI_CMD_GET_CSS_PWR))
		return;

	test_begin(TEST_CSS_INFO);
	scpi_prepare_msg(&msg, SCPI_CMD_GET_CSS_PWR);
	test_send_request(&msg);
	test_assert(msg.status == SCPI_OK);
	/* Assert that there is at least one cluster. */
	test_assert(msg.size >= 2);
	/* Each descriptor is 2 bytes long. */
	test_assert(msg.size % 2 == 0);
	/* Assert that all clusters are on. */
	for (size_t i = 0; i < msg.size / 2; ++i)
		test_assert((((uint16_t *)msg.payload)[i] & 0x0f) == 0);
	test_complete(TEST_CSS_INFO);
}

/*
 * Test: DVFS.
 */
static void
try_dvfs(void)
{
	struct scpi_msg msg;
	uint8_t domains = 0, opps;

	/* Skip this test if the required commands are not available. */
	if (!scpi_has_command(SCPI_CMD_GET_DVFS_CAP))
		return;

	/* Get the number of DVFS domains. */
	test_begin(TEST_DVFS_CAP);
	scpi_prepare_msg(&msg, SCPI_CMD_GET_DVFS_CAP);
	test_send_request(&msg);
	test_assert(msg.status == SCPI_OK);
	test_assert(msg.size == 1);
	domains = ((uint8_t *)msg.payload)[0];
	test_complete(TEST_DVFS_CAP);

	/* If the firmware has DVFS domains, it must fully support DVFS. */
	if (domains > 0) {
		test_begin(TEST_DVFS_CMDS);
		test_assert(scpi_has_command(SCPI_CMD_GET_DVFS_INFO));
		test_assert(scpi_has_command(SCPI_CMD_SET_DVFS));
		test_assert(scpi_has_command(SCPI_CMD_GET_DVFS));
		test_complete(TEST_DVFS_CMDS);
	}

	for (volatile uint8_t i = 0; i < domains; ++i) {
		test_begin(TEST_DVFS_INFO);
		scpi_prepare_msg(&msg, SCPI_CMD_GET_DVFS_INFO);
		/* Send the domain ID. */
		msg.size = 1;
		((uint8_t *)msg.payload)[0] = i;
		test_send_request(&msg);
		test_assert(msg.status == SCPI_OK);
		test_assert(msg.size >= 4);
		/* Assert that we got the same ID back. */
		test_assert(((uint8_t *)msg.payload)[0] == i);
		/* Get the number of operating performance points. */
		opps = ((uint8_t *)msg.payload)[1];
		/* Ensure the number of OPPs matches the message size. */
		test_assert(opps > 0);
		test_assert(msg.size == 8 * opps + 4);
		test_complete(TEST_DVFS_INFO);

		/* Cycle through all of the OPPs. */
		test_begin(TEST_DVFS_CTRL);
		for (uint8_t j = 0; j < opps; ++j) {
			/* Set DVFS domain i to OPP j. */
			scpi_prepare_msg(&msg, SCPI_CMD_SET_DVFS);
			msg.size = 2;
			((uint8_t *)msg.payload)[0] = i;
			((uint8_t *)msg.payload)[1] = j;
			test_send_request(&msg);
			test_assert(msg.status == SCPI_OK);
			test_assert(msg.size == 0);

			/* Read back the OPP index and ensure it matches. */
			scpi_prepare_msg(&msg, SCPI_CMD_GET_DVFS);
			msg.size = 1;
			((uint8_t *)msg.payload)[0] = i;
			test_send_request(&msg);
			test_assert(msg.status == SCPI_OK);
			test_assert(msg.size == 1);
			test_assert(((uint8_t *)msg.payload)[0] == j);
		}
		test_complete(TEST_DVFS_CTRL);
	}

	/* Test the failure case of sending an invalid domain ID. */
	test_begin(TEST_DVFS_INFO);
	scpi_prepare_msg(&msg, SCPI_CMD_GET_DVFS_INFO);
	msg.size = 1;
	((uint8_t *)msg.payload)[0] = domains;
	test_send_request(&msg);
	test_assert(msg.status == SCPI_E_PARAM);
	test_complete(TEST_DVFS_INFO);
}

/*
 * Test: Power supplies.
 */
static void
try_psus(void)
{
	struct scpi_msg msg;
	uint16_t psus = 0;

	/* Skip this test if the required commands are not available. */
	if (!scpi_has_command(SCPI_CMD_GET_PSU_CAP))
		return;

	/* Get the number of clocks. */
	test_begin(TEST_PSU_CAP);
	scpi_prepare_msg(&msg, SCPI_CMD_GET_PSU_CAP);
	test_send_request(&msg);
	test_assert(msg.status == SCPI_OK);
	test_assert(msg.size == 2);
	psus = ((uint16_t *)msg.payload)[0];
	test_complete(TEST_PSU_CAP);

	/* If the firmware has clocks, it must fully support clock control. */
	if (psus > 0) {
		test_begin(TEST_PSU_CMDS);
		test_assert(scpi_has_command(SCPI_CMD_GET_PSU_INFO));
		test_assert(scpi_has_command(SCPI_CMD_SET_PSU));
		test_assert(scpi_has_command(SCPI_CMD_GET_PSU));
		test_complete(TEST_PSU_CMDS);
	}

	for (volatile uint16_t i = 0; i < psus; ++i) {
		uint8_t  flags;
		uint32_t min_voltage, max_voltage, voltage;

		test_begin(TEST_PSU_INFO);
		scpi_prepare_msg(&msg, SCPI_CMD_GET_PSU_INFO);
		/* Send the PSU ID. */
		msg.size = 2;
		((uint16_t *)msg.payload)[0] = i;
		test_send_request(&msg);
		test_assert(msg.status == SCPI_OK);
		test_assert(msg.size >= 12);
		/* Assert that we got the same ID back. */
		test_assert(((uint16_t *)msg.payload)[0] == i);
		/* Assert that there are no reserved flags. */
		flags = ((uint16_t *)msg.payload)[1];
		test_assert((flags & ~(FLAG_READABLE | FLAG_WRITABLE)) == 0);
		/* Assert that the minimum and maximum voltages make sense. */
		min_voltage = ((uint32_t *)msg.payload)[1];
		max_voltage = ((uint32_t *)msg.payload)[2];
		test_assert(max_voltage >= min_voltage);
		test_complete(TEST_PSU_INFO);

		/* Set an invalid value (over the maximum). */
		scpi_prepare_msg(&msg, SCPI_CMD_SET_PSU);
		msg.size = 8;
		((uint32_t *)msg.payload)[0] = i;
		((uint32_t *)msg.payload)[1] = max_voltage + 1000;
		test_send_request(&msg);
		/* Assert that the firmware respects its permission flags. */
		if (flags & FLAG_WRITABLE)
			test_assert(msg.status == SCPI_E_RANGE);
		else
			test_assert(msg.status == SCPI_E_ACCESS);
		test_assert(msg.size == 0);

		/* Get the current value, for setting below. */
		test_begin(TEST_PSU_CTRL);
		scpi_prepare_msg(&msg, SCPI_CMD_GET_PSU);
		msg.size = 2;
		((uint16_t *)msg.payload)[0] = i;
		test_send_request(&msg);
		/* Assert that the firmware respects its permission flags. */
		if (!(flags & FLAG_READABLE)) {
			test_assert(msg.status == SCPI_E_ACCESS);
			test_complete(TEST_PSU_CTRL);
			continue;
		}
		test_assert(msg.status == SCPI_OK);
		test_assert(msg.size == 4);
		voltage = ((uint32_t *)msg.payload)[0];
		test_assert(voltage >= min_voltage);
		test_assert(voltage <= max_voltage);

		/* Setting arbitrary values is unsafe; set what was read. */
		scpi_prepare_msg(&msg, SCPI_CMD_SET_PSU);
		msg.size = 8;
		((uint32_t *)msg.payload)[0] = i;
		((uint32_t *)msg.payload)[1] = voltage;
		test_send_request(&msg);
		/* Assert that the firmware respects its permission flags. */
		if (flags & FLAG_WRITABLE)
			test_assert(msg.status == SCPI_OK);
		else
			test_assert(msg.status == SCPI_E_ACCESS);
		test_assert(msg.size == 0);
		test_complete(TEST_PSU_CTRL);
	}

	/* Test the failure case of sending an invalid PSU ID. */
	test_begin(TEST_PSU_INFO);
	scpi_prepare_msg(&msg, SCPI_CMD_GET_PSU_INFO);
	msg.size = 2;
	((uint16_t *)msg.payload)[0] = psus;
	test_send_request(&msg);
	test_assert(msg.status == SCPI_E_PARAM);
	test_complete(TEST_PSU_INFO);
}

/*
 * Test: Sensors.
 */
static void
try_sensors(void)
{
	struct scpi_msg msg;
	uint16_t sensors = 0;

	/* Skip this test if the required commands are not available. */
	if (!scpi_has_command(SCPI_CMD_GET_SENSOR_CAP))
		return;

	/* Get the number of sensors. */
	test_begin(TEST_SENSOR_CAP);
	scpi_prepare_msg(&msg, SCPI_CMD_GET_SENSOR_CAP);
	test_send_request(&msg);
	test_assert(msg.status == SCPI_OK);
	test_assert(msg.size == 2);
	sensors = ((uint16_t *)msg.payload)[0];
	test_complete(TEST_SENSOR_CAP);

	/* If the firmware has clocks, it must fully support clock control. */
	if (sensors > 0) {
		test_begin(TEST_SENSOR_CMDS);
		test_assert(scpi_has_command(SCPI_CMD_GET_SENSOR_INFO));
		test_assert(scpi_has_command(SCPI_CMD_GET_SENSOR));
		test_complete(TEST_SENSOR_CMDS);
	}

	for (volatile uint16_t i = 0; i < sensors; ++i) {
		uint8_t  class;
		uint32_t value;

		test_begin(TEST_SENSOR_INFO);
		scpi_prepare_msg(&msg, SCPI_CMD_GET_SENSOR_INFO);
		/* Send the sensor ID. */
		msg.size = 2;
		((uint16_t *)msg.payload)[0] = i;
		test_send_request(&msg);
		test_assert(msg.status == SCPI_OK);
		test_assert(msg.size >= 4);
		/* Assert that we got the same ID back. */
		test_assert(((uint16_t *)msg.payload)[0] == i);
		/* Assert that the class is valid. */
		class = ((uint8_t *)msg.payload)[2];
		test_assert(class < SENSOR_CLASS_COUNT);
		/* Assert that the triggers are valid. */
		test_assert((msg.payload[3] & ~(BIT(0) | BIT(1))) == 0);
		test_complete(TEST_SENSOR_INFO);

		/* Check the current value for sanity. */
		test_begin(TEST_SENSOR_READ);
		scpi_prepare_msg(&msg, SCPI_CMD_GET_SENSOR);
		msg.size = 2;
		((uint16_t *)msg.payload)[0] = i;
		test_send_request(&msg);
		test_assert(msg.status == SCPI_OK);
		test_assert(msg.size == 8);
		value = ((uint32_t *)msg.payload)[0];
		/* Ensure temperature readings are below 100°C. */
		if (class == SENSOR_CLASS_TEMPERATURE)
			test_assert(value < 100000);
		/* The firmware does not support more than 32-bit output. */
		test_assert(((uint32_t *)msg.payload)[1] == 0);
		test_complete(TEST_SENSOR_READ);
	}
}

/*
 * Test: System power.
 */
static void
try_sys_power(void)
{
	struct scpi_msg msg;

	/* Positive tests would reset the machine; only test failure cases. */
	test_begin(TEST_SYS_POWER);
	scpi_prepare_msg(&msg, SCPI_CMD_SET_SYS_PWR);
	msg.size = 1;
	/* 4 is not a valid system power state. */
	((uint8_t *)msg.payload)[0] = 4;
	test_send_request(&msg);
	test_assert(msg.status == SCPI_E_PARAM);

	scpi_prepare_msg(&msg, SCPI_CMD_SET_SYS_PWR);
	msg.size = 1;
	/* 20 is not a valid system power state. */
	((uint8_t *)msg.payload)[0] = 20;
	test_send_request(&msg);
	test_assert(msg.status == SCPI_E_PARAM);
	test_complete(TEST_SYS_POWER);
}

int
main(int argc, char *argv[])
{
	void *mbox_map, *sram_map;
	int fd;

	static_assert(sizeof(struct scpi_msg) == SCPI_MESSAGE_SIZE,
	              "struct scpi_msg does not have the correct size");

	if (argc >= 2) {
		puts("ARISC firmware tester for " CONFIG_PLATFORM);
		printf("usage: %s [--help]\n", argv[0]);
		return strcmp("--help", argv[1]) ? EXIT_FAILURE : EXIT_SUCCESS;
	}

	/* Map the SCPI shared memory and the message box. */
	fd = open("/dev/mem", O_RDWR | O_SYNC);
	if (fd < 0) {
		perror("Failed to open /dev/mem");
		return EXIT_FAILURE;
	}
	mbox_map = mmap(NULL, PAGESIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd,
	                PAGE_BASE(DEV_MSGBOX));
	if (mbox_map == MAP_FAILED) {
		perror("Failed to mmap MSGBOX");
		return EXIT_FAILURE;
	}
	sram_map = mmap(NULL, PAGESIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd,
	                PAGE_BASE(SRAM_A2_OFFSET + SCPI_MEM_BASE));
	if (sram_map == MAP_FAILED) {
		perror("Failed to mmap SRAM A2");
		return EXIT_FAILURE;
	}
	close(fd);

	/* Correct the addresses for mmap requiring page alignment. */
	mbox = (uintptr_t)mbox_map + PAGE_OFFSET(DEV_MSGBOX);
	sram = (uintptr_t)sram_map + PAGE_OFFSET(SRAM_A2_OFFSET +
	                                         SCPI_MEM_BASE);

	/* Set up the fatal error handler. */
	if (sigsetjmp(main_buf, 0)) {
		test_summary();
		return EXIT_FAILURE;
	}

	/* Run all tests. */
	try_boot();
	try_basic();
	try_clocks();
	try_css_power();
	try_dvfs();
	try_psus();
	try_sensors();
	try_sys_power();

	/* Display a summary of the tests. */
	test_summary();

	/* Clean up. */
	munmap(mbox_map, PAGESIZE);
	munmap(sram_map, PAGESIZE);

	return EXIT_SUCCESS;
}
