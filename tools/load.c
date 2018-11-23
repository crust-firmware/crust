/*
 * Copyright © 2017-2018 The Crust Firmware Authors.
 * SPDX-License-Identifier: BSD-3-Clause OR GPL-2.0-only
 */

#include <fcntl.h>
#include <limits.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>

#include <config.h>
#include <mmio.h>
#include <util.h>
#include <platform/devices.h>
#include <platform/memory.h>

#ifndef PAGESIZE
#define PAGESIZE          0x1000
#endif
#define PAGE_BASE(addr)   ((addr) & ~(PAGESIZE - 1))
#define PAGE_OFFSET(addr) ((addr) & (PAGESIZE - 1))

#define SRAM_ARM_OFFSET   0x40000

static inline uint32_t
mmio_getbits32(uintptr_t addr, uint32_t get)
{
	return mmio_read32(addr) & get;
}

int
main(int argc, char *argv[])
{
	struct stat st;
	char     *file;
	char     *sram;
	int       fd;
	uintptr_t r_cpucfg;
	void     *r_cpucfg_map;

	if (argc < 2 || strcmp("--help", argv[1]) == 0) {
		puts("ARISC firmware loader for " CONFIG_PLATFORM);
		printf("usage: %s [--help] --reset | <filename>\n", argv[0]);
		return argc < 2 ? EXIT_FAILURE : EXIT_SUCCESS;
	}

	fd = open("/dev/mem", O_RDWR | O_SYNC);
	if (fd < 0) {
		perror("Failed to open /dev/mem");
		return EXIT_FAILURE;
	}
	r_cpucfg_map = mmap(NULL, PAGESIZE, PROT_READ | PROT_WRITE, MAP_SHARED,
	                    fd, PAGE_BASE(DEV_R_CPUCFG));
	if (r_cpucfg_map == MAP_FAILED) {
		perror("Failed to mmap R_CPUCFG");
		return EXIT_FAILURE;
	}
	sram = mmap(NULL, VECTORS_SIZE + FIRMWARE_SIZE, PROT_READ | PROT_WRITE,
	            MAP_SHARED, fd, SRAM_ARM_OFFSET + PAGE_BASE(VECTORS_BASE));
	if (sram == MAP_FAILED) {
		perror("Failed to mmap SRAM A2");
		return EXIT_FAILURE;
	}
	close(fd);

	r_cpucfg = (uintptr_t)r_cpucfg_map + PAGE_OFFSET(DEV_R_CPUCFG);

	if (strcmp("--reset", argv[1]) == 0) {
		if (!mmio_getbits32(r_cpucfg, BIT(0))) {
			puts("ARISC is already in reset");
			return EXIT_SUCCESS;
		}
		puts("Asserting ARISC reset");
		mmio_clearbits32(r_cpucfg, BIT(0));
		if (mmio_getbits32(r_cpucfg, BIT(0))) {
			puts("Failed to assert ARISC reset");
			return EXIT_FAILURE;
		}
		return EXIT_SUCCESS;
	}

	fd = open(argv[1], O_RDONLY);
	if (fd < 0) {
		perror("Failed to open firmware file");
		return EXIT_FAILURE;
	}
	if (fstat(fd, &st)) {
		perror("Failed to stat firmware file");
		return EXIT_FAILURE;
	}
	if (st.st_size <= VECTORS_SIZE) {
		puts("Firmware file is too small");
		return EXIT_FAILURE;
	}
	if (st.st_size > VECTORS_SIZE + FIRMWARE_SIZE) {
		puts("Firmware will not fit in available SRAM");
		return EXIT_FAILURE;
	}
	file = mmap(NULL, st.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
	if (file == MAP_FAILED) {
		perror("Failed to mmap firmware file");
		return EXIT_FAILURE;
	}
	close(fd);

	puts("Asserting ARISC reset");
	mmio_clearbits32(r_cpucfg, BIT(0));
	if (mmio_getbits32(r_cpucfg, BIT(0))) {
		puts("Failed to assert ARISC reset");
		return EXIT_FAILURE;
	}
	printf("Writing firmware (%jd/%d bytes used)\n",
	       (intmax_t)(st.st_size - VECTORS_SIZE), FIRMWARE_SIZE);
	memcpy(sram, file, st.st_size);
	msync(sram, st.st_size, MS_SYNC);
	puts("Deasserting ARISC reset");
	mmio_setbits32(r_cpucfg, BIT(0));
	if (!mmio_getbits32(r_cpucfg, BIT(0))) {
		puts("Failed to deassert ARISC reset");
		return EXIT_FAILURE;
	}

	munmap(file, st.st_size);
	munmap(r_cpucfg_map, PAGESIZE);
	munmap(sram, VECTORS_SIZE + FIRMWARE_SIZE);

	return EXIT_SUCCESS;
}
