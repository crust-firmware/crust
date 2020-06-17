/*
 * Copyright © 2020 The Crust Firmware Authors.
 * SPDX-License-Identifier: BSD-3-Clause OR GPL-2.0-only
 */

#include <debug.h>
#include <mmio.h>
#include <regmap.h>
#include <serial.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <system.h>
#include <mfd/axp20x.h>

#define MAX_LENGTH 19 /* "m xxxxxxxx xxxxxxxx" */

static char buffer[MAX_LENGTH + 1];
static uint8_t length;

static bool
parse_hex(const char **str, uint32_t *num)
{
	uint32_t n    = 0;
	const char *s = *str;
	bool ret      = false;

	/* Skip spaces. */
	while (*s == ' ')
		++s;

	/* At least one space must precede the number. */
	if (s == *str)
		return false;

	/* Consume as many hex digits as found. */
	for (;;) {
		uint32_t c = *s | 0x20;
		uint32_t digit;

		if (c - '0' < 10)
			digit = c - '0';
		else if (c - 'a' < 6)
			digit = c - ('a' - 10);
		else
			break;
		++s;
		n   = n << 4 | digit;
		ret = true;
	}
	*str = s;
	*num = n;

	return ret;
}

static void
run_command(const char *cmd)
{
	uint32_t addr, len;

	switch (*cmd++) {
	case 'd':
		/* Hex dump: "d xxxxxxxx xxxxxxxx", arguments in bare hex. */
		if (parse_hex(&cmd, &addr) && parse_hex(&cmd, &len))
			hexdump(addr, len);
		return;
	case 'm':
		/* MMIO: "m xxxxxxxx" or "m xxxxxxxx xxxxxxxx", bare hex. */
		if (parse_hex(&cmd, &addr)) {
			uint32_t val;

			if (parse_hex(&cmd, &val))
				mmio_write_32(addr, val);

			log("%08x: %08x\n", addr, mmio_read_32(addr));
		}
		return;
#if CONFIG(DEBUG_MONITOR_PMIC)
	case 'p':
		/* PMIC: "p xx" or "p xx xx", bare hex. */
		if (parse_hex(&cmd, &addr)) {
			const struct regmap *map = &axp20x.map;
			uint32_t val32;
			uint8_t  val8;

			if (regmap_user_probe(map))
				return;

			if (parse_hex(&cmd, &val32))
				regmap_write(map, addr, val32);

			regmap_read(map, addr, &val8);
			log("%02x: %02x\n", addr, val8);

			regmap_user_release(map);
		}
		return;
#endif
	case 's':
		/* System: "sb", "sr", or "sw". */
		if (*cmd == 'b')
			system_reboot();
		else if (*cmd == 'r')
			system_reset();
		else if (*cmd == 'w')
			system_wakeup();
		return;
	}
}

void
debug_monitor(void)
{
	unsigned char c;

	if (system_is_running())
		return;
	if (!(c = serial_getc()))
		return;

	if (c < ' ' || length == MAX_LENGTH) {
		serial_putc('\n');
		if (c == '\r') {
			buffer[length] = 0;
			run_command(buffer);
		}
		serial_puts("> ");
		length = 0;
	} else {
		serial_putc(c);
		buffer[length++] = c;
	}
}
