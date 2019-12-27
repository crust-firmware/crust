/*
 * Copyright © 2017-2019 The Crust Firmware Authors.
 * SPDX-License-Identifier: BSD-3-Clause OR GPL-2.0-only
 */

#ifndef PLATFORM_IRQ_H
#define PLATFORM_IRQ_H

#define IRQ_NMI      0x00
#define IRQ_R_TIMER0 0x01
#define IRQ_R_TIMER1 0x02
#define IRQ_R_TIMER2 0x03
#define IRQ_R_TIMER3 0x04
#define IRQ_R_ALARM0 0x05
#define IRQ_R_ALARM1 0x06
#define IRQ_R_WDOG   0x07
#define IRQ_R_TWD    0x08
#define IRQ_R_PIO_PL 0x09
#define IRQ_R_UART   0x0a
#define IRQ_R_I2C    0x0b
#define IRQ_R_CIR_RX 0x0d
#define IRQ_R_OWC    0x0e
#define IRQ_R_PIO_PM 0x0f

#endif /* PLATFORM_IRQ_H */
