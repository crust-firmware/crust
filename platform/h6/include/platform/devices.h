/*
 * Copyright © 2017-2019 The Crust Firmware Authors.
 * SPDX-License-Identifier: BSD-3-Clause OR GPL-2.0-only
 */

#ifndef PLATFORM_DEVICES_H
#define PLATFORM_DEVICES_H

#define DEV_DE        0x01000000
#define DEV_DEBUG     0x01420000
#define DEV_GPU       0x01800000
#define DEV_CE_NS     0x01904000
#define DEV_CE_S      0x01904800
#define DEV_EMCE      0x01905000
#define DEV_KEYMEM    0x01908000
#define DEV_VP9       0x01c00000
#define DEV_VE        0x01c0e000
#define DEV_SYSCON    0x03000000
#define DEV_CCU       0x03001000
#define DEV_DMA       0x03002000
#define DEV_MSGBOX    0x03003000
#define DEV_SPINLOCK  0x03004000
#define DEV_HSTIMER   0x03005000
#define DEV_SID       0x03006000
#define DEV_TIMER     0x03009000
#define DEV_PWM       0x0300a000
#define DEV_PIO       0x0300b000
#define DEV_PSI       0x0300c000
#define DEV_DCU       0x03010000
#define DEV_SCU       0x03020000
#define DEV_GICD      0x03021000
#define DEV_GICC      0x03022000
#define DEV_IOMMU     0x030f0000
#define DEV_DRAMCTL   0x04002000
#define DEV_NAND      0x04011000
#define DEV_MMC0      0x04020000
#define DEV_MMC1      0x04021000
#define DEV_MMC2      0x04022000
#define DEV_UART0     0x05000000
#define DEV_UART1     0x05000400
#define DEV_UART2     0x05000800
#define DEV_UART3     0x05000c00
#define DEV_I2C0      0x05002000
#define DEV_I2C1      0x05002400
#define DEV_I2C2      0x05002800
#define DEV_I2C3      0x05002c00
#define DEV_SCR0      0x05005000
#define DEV_SCR1      0x05005400
#define DEV_SPI0      0x05010000
#define DEV_SPI1      0x05011000
#define DEV_EMAC      0x05020000
#define DEV_TS0       0x05060000
#define DEV_THS       0x05070400
#define DEV_CIR_TX    0x05061000
#define DEV_I2S3      0x0508f000
#define DEV_I2S0      0x05090000
#define DEV_I2S1      0x05091000
#define DEV_I2S2      0x05092000
#define DEV_SPDIF     0x05093000
#define DEV_DMIC      0x05095000
#define DEV_AHUB      0x05097000
#define DEV_USB0      0x05100000
#define DEV_USB1      0x05200000
#define DEV_USB3      0x05311000
#define DEV_PCIE      0x05400000
#define DEV_HDMI      0x06000000
#define DEV_TCON_TOP  0x06510000
#define DEV_TCON0     0x06511000
#define DEV_TCON1     0x06515000
#define DEV_CSI       0x06620000
#define DEV_RTC       0x07000000
#define DEV_R_CPUCFG  0x07000400
#define DEV_R_PRCM    0x07010000
#define DEV_R_TIMER   0x07020000
#define DEV_R_WDOG    0x07020400
#define DEV_R_TWD     0x07020800
#define DEV_R_PWM     0x07020c00
#define DEV_R_INTC    0x07021000
#define DEV_R_PIO     0x07022000
#define DEV_R_CIR_RX  0x07040000
#define DEV_R_W1      0x07040400
#define DEV_R_UART    0x07080000
#define DEV_R_I2C     0x07081400
#define DEV_CPUSYSCFG 0x08100000
#define DEV_CNT_R     0x08110000
#define DEV_CNT_C     0x08120000
#define DEV_CPUCFG    0x09010000
#define DEV_CPU_BIST  0x09020000

#endif /* PLATFORM_DEVICES_H */
