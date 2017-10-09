/*
 * Copyright © 2017-2018 Samuel Holland <samuel@sholland.org>
 * SPDX-License-Identifier: (BSD-3-Clause OR GPL-2.0)
 */

#define _XOPEN_SOURCE 700
#include <fcntl.h>
#include <limits.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <platform/devices.h>
#include <platform/memory.h>
#include <sys/mman.h>
#include <sys/stat.h>

#define SRAM_ARM_OFFSET   0x40000

#define PAGE_BASE(addr)   ((addr) & ~(PAGE_SIZE - 1))
#define PAGE_OFFSET(addr) ((addr) & (PAGE_SIZE - 1))
#ifndef PAGE_SIZE
#define PAGE_SIZE         0x1000
#endif

static inline uint32_t
mmio_read32(char *addr)
{
	return *(volatile uint32_t *)addr;
}

static inline void
mmio_write32(char *addr, uint32_t value)
{
	*(volatile uint32_t *)addr = value;
}

int
main(int argc, char *argv[])
{
	struct stat     st;
	struct timespec ts = { .tv_sec = 0, .tv_nsec = 10000000 };
	char *cpu;
	char *file;
	char *sram;
	int   fd;

	if (argc < 2 || strcmp("--help", argv[1]) == 0) {
		puts("ARISC firmware loader for " PLATFORM);
		printf("usage: %s [--help] --reset | <filename>\n", argv[0]);
		return argc < 2 ? EXIT_FAILURE : EXIT_SUCCESS;
	}

	fd = open("/dev/mem", O_RDWR | O_SYNC);
	if (fd < 0) {
		perror("Failed to open /dev/mem");
		return EXIT_FAILURE;
	}
	cpu = mmap(NULL, PAGE_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd,
	           PAGE_BASE(DEV_R_CPUCFG));
	if (cpu == MAP_FAILED) {
		perror("Failed to mmap R_CPUCFG");
		return EXIT_FAILURE;
	}
	sram = mmap(NULL, VECTORS_SIZE + FIRMWARE_SIZE, PROT_READ | PROT_WRITE,
	            MAP_SHARED, fd, SRAM_ARM_OFFSET + VECTORS_BASE);
	if (sram == MAP_FAILED) {
		perror("Failed to mmap SRAM A2");
		return EXIT_FAILURE;
	}
	close(fd);

	if (strcmp("--reset", argv[1]) == 0) {
		puts("Asserting ARISC reset");
		mmio_write32(cpu + PAGE_OFFSET(DEV_R_CPUCFG), 0);
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
	mmio_write32(cpu + PAGE_OFFSET(DEV_R_CPUCFG), 0);
	nanosleep(&ts, NULL);
	printf("Writing firmware (%ld/%d bytes used)\n",
	       st.st_size - VECTORS_SIZE, FIRMWARE_SIZE);
	memcpy(sram, file, st.st_size);
	msync(sram, st.st_size, MS_SYNC);
	puts("Deasserting ARISC reset");
	nanosleep(&ts, NULL);
	mmio_write32(cpu + PAGE_OFFSET(DEV_R_CPUCFG), 1);

	munmap(cpu, PAGE_SIZE);
	munmap(file, st.st_size);
	munmap(sram, VECTORS_SIZE + FIRMWARE_SIZE);

	return EXIT_SUCCESS;
}
