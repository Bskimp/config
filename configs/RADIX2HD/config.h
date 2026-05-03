/*
 * This file is part of Cleanflight and Betaflight.
 *
 * Cleanflight and Betaflight are free software. You can redistribute
 * this software and/or modify this software under the terms of the
 * GNU General Public License as published by the Free Software
 * Foundation, either version 3 of the License, or (at your option)
 * any later version.
 *
 * Cleanflight and Betaflight are distributed in the hope that they
 * will be useful, but WITHOUT ANY WARRANTY; without even the implied
 * warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this software.
 *
 * If not, see <http://www.gnu.org/licenses/>.
 */

/*
 * BrainFPV RADIX 2 HD config for Betaflight master.
 *
 * Adapted from BrainFPV/betaflight 4.5.1-BRAIN-2 with cross-references to
 * ArduPilot's RADIX2HD hwdef and live board CLI dump (2026-05-02).
 * SPRACINGH7EXTREME used as structural template.
 *
 * Companion files: config.c (.bl_header_section struct definition).
 * Companion linker: src/platform/STM32/link/stm32_h750_brainfpv_xip.ld
 * Companion platform patches: persistent.h + persistent.c (USE_BRAINFPV_BL).
 */

#pragma once

#define FC_TARGET_MCU       STM32H750

#define BOARD_NAME          RADIX2HD
#define MANUFACTURER_ID     BRFP

#define TARGET_BOARD_IDENTIFIER "RDX2HD"
#define USBD_PRODUCT_STRING     "BrainFPV RADIX 2 HD"

// 16 MHz external crystal (confirmed via BrainFPV target.mk @ 4.5.1-BRAIN-2).
// Build system reads SYSTEM_HSE_MHZ via mk/config.mk and passes -DHSE_VALUE=
// on the compile command line, so we set the MHz form here, not HSE_VALUE.
#define SYSTEM_HSE_MHZ      16

/* ------------------------------------------------------------------------
 * Firmware layout — XIP from QSPI
 *
 * The BrainFPV bootloader expects firmware at 0x90400000 (memory-mapped
 * QSPI, +4 MB from QSPI base) and runs in-place. This is different from
 * BF's standard EXST flow which copies firmware to RAM at 0x24010000.
 * Our linker script places .text/.rodata directly in QSPI; .data/.bss
 * still live in RAM as usual.
 * ------------------------------------------------------------------------ */
#define FC_VMA_ADDRESS              0x90400000
#define CONFIG_IN_EXTERNAL_FLASH
#define USE_FIRMWARE_PARTITION
#define EEPROM_SIZE                 8192
// Note: FLASH_PAGE_SIZE intentionally NOT defined here. Master's
// STM32H750/target.h sets it to 0x20000 (128 KB internal-flash sector size,
// the H750 hardware value). The W25Q128 QSPI's 64 KB sector size is
// auto-detected at runtime via JEDEC ID by the flash driver.

/* ------------------------------------------------------------------------
 * Bootloader handshake — two independent mechanisms
 *
 * (1) Firmware-image validation: the magic 0x785E9A14 lives in a
 *     .bl_header_section flash region; the bootloader reads it before
 *     jumping to firmware. See companion config.c for the struct, and
 *     the custom linker script for the section placement.
 *
 * (2) Stay-in-DFU signal: when systemResetToBootloader() writes
 *     RESET_BOOTLOADER_REQUEST_ROM (==1) to PERSISTENT_OBJECT_RESET_REASON,
 *     a USE_BRAINFPV_BL #ifdef block in persistent.c dual-writes the same
 *     value to PERSISTENT_OBJECT_BRAINFPV_BL (= RTC_BKP_DR6). The bootloader
 *     reads BKP_DR6 on next boot to decide whether to stay in DFU mode.
 *     Mirrors SPRACINGH7EXTREME's USE_SPRACING_PERSISTENT_RTC_WORKAROUND.
 * ------------------------------------------------------------------------ */
#define USE_BRAINFPV_BL
#define BRAINFPV_BL_HEADER_MAGIC    0x785E9A14

/* ------------------------------------------------------------------------
 * Status LED / beeper
 *
 * BrainFPV had an RGB status LED driven by TIM14_CH1 (PA7) + TIM15_CH1/2
 * (PE5/PE6) for software PWM color mixing. Mainline has no equivalent;
 * exposed as plain single-color GPIO LEDs.
 * ------------------------------------------------------------------------ */
#define LED0_PIN            PA7
#define LED0_INVERTED
#define LED1_PIN            PE5
#define LED1_INVERTED

#define USE_BEEPER
#define BEEPER_PIN          PE4
#define BEEPER_INVERTED

/* ------------------------------------------------------------------------
 * VTX 9 V pit switch — BOXPIT permanent ID 40
 * ------------------------------------------------------------------------ */
#define USE_PINIO
#define USE_PINIOBOX
#define PINIO1_PIN          PC14
#define PINIO1_BOX          40    // BOXPIT — pgResetFn_pinioBoxConfig consumes

/* ------------------------------------------------------------------------
 * UARTs
 *
 * UART layout from BrainFPV target.h, cross-checked against ArduPilot
 * hwdef + live board resource list. UART7 is RX-only (HD VTX RC input).
 * ------------------------------------------------------------------------ */
#define UART1_RX_PIN        PB15
#define UART1_TX_PIN        PB14
#define UART2_RX_PIN        PD6
#define UART2_TX_PIN        PD5
#define UART3_RX_PIN        PB11
#define UART3_TX_PIN        PD8
#define UART4_RX_PIN        PB8
#define UART4_TX_PIN        PA0
#define UART5_RX_PIN        PB12
#define UART5_TX_PIN        PB13
#define UART6_RX_PIN        PC7   // shared with SERVO2
#define UART6_TX_PIN        PC6   // shared with SERVO1
#define UART7_RX_PIN        PA8   // RC input from HD VTX (RX-only)
#define UART7_TX_PIN        NONE

#define USE_VCP
#define USE_USB_ID
#define USE_USB_DETECT
#define USB_DETECT_PIN      PA9

/* ------------------------------------------------------------------------
 * SPI — single bus, BMI270 only
 * ------------------------------------------------------------------------ */
#define USE_SPI
#define USE_SPI_DEVICE_1
#define SPI1_SCK_PIN        PA5
#define SPI1_SDI_PIN        PB4
#define SPI1_SDO_PIN        PD7
#define SPI1_NSS_PIN        PD3   // == GYRO_1_CS_PIN

/* ------------------------------------------------------------------------
 * I2C — external compass / airspeed / DPS310 baro
 * ------------------------------------------------------------------------ */
#define USE_I2C
#define USE_I2C_DEVICE_1
#define I2C1_SCL_PIN        PB6
#define I2C1_SDA_PIN        PB7
#define I2C_DEVICE          I2CDEV_1
#define MAG_I2C_INSTANCE    I2CDEV_1

/* ------------------------------------------------------------------------
 * QSPI flash — W25Q128FV (JEDEC 0x00016018, confirmed via live dump)
 * ------------------------------------------------------------------------ */
#define USE_QUADSPI
#define USE_QUADSPI_DEVICE_1
#define QUADSPI1_SCK_PIN        PB2
#define QUADSPI1_BK1_IO0_PIN    PD11
#define QUADSPI1_BK1_IO1_PIN    PD12
#define QUADSPI1_BK1_IO2_PIN    PE2
#define QUADSPI1_BK1_IO3_PIN    PA1
#define QUADSPI1_BK1_CS_PIN     PB10
#define QUADSPI1_BK2_IO0_PIN    NONE
#define QUADSPI1_BK2_IO1_PIN    NONE
#define QUADSPI1_BK2_IO2_PIN    NONE
#define QUADSPI1_BK2_IO3_PIN    NONE
#define QUADSPI1_BK2_CS_PIN     NONE
#define QUADSPI1_MODE           QUADSPI_MODE_BK1_ONLY
#define QUADSPI1_CS_FLAGS       (QUADSPI_BK1_CS_HARDWARE | QUADSPI_BK2_CS_NONE | QUADSPI_CS_MODE_LINKED)
#define FLASH_QUADSPI_INSTANCE  QUADSPI

#define USE_FLASH
#define USE_FLASH_W25Q128FV
#define USE_FLASH_QUADSPI
#define USE_FLASHFS

/* ------------------------------------------------------------------------
 * SD card (SDIO)
 * ------------------------------------------------------------------------ */
#define USE_SDCARD
#define USE_SDCARD_SDIO
#define SDCARD_DETECT_PIN       PD9
#define SDCARD_DETECT_INVERTED
#define SDIO_DEVICE             SDIODEV_1
#define SDIO_USE_4BIT           true
#define SDIO_CK_PIN             PC12
#define SDIO_CMD_PIN            PD2
#define SDIO_D0_PIN             PC8
#define SDIO_D1_PIN             PC9
#define SDIO_D2_PIN             PC10
#define SDIO_D3_PIN             PC11

// Default to onboard QSPI (matches BrainFPV stock config + live HD dump)
#define DEFAULT_BLACKBOX_DEVICE BLACKBOX_DEVICE_FLASH

/* ------------------------------------------------------------------------
 * Gyro / accel — BMI270 on SPI1
 * ------------------------------------------------------------------------ */
#define USE_EXTI
#define USE_GYRO
#define USE_ACC
#define USE_GYRO_EXTI
#define USE_SPI_GYRO
#define USE_ACCGYRO_BMI270

#define GYRO_1_EXTI_PIN         PB3
#define GYRO_1_CS_PIN           PD3
#define GYRO_1_SPI_INSTANCE     SPI1
#define GYRO_1_ALIGN            CW0_DEG
#define DEFAULT_GYRO_TO_USE     GYRO_CONFIG_USE_GYRO_1

/* ------------------------------------------------------------------------
 * Baro — DPS310 on I2C1
 * ------------------------------------------------------------------------ */
#define USE_BARO
#define USE_BARO_DPS310
#define BARO_I2C_INSTANCE       I2CDEV_1

/* ------------------------------------------------------------------------
 * Mag — none on board, all external via I2C
 * ------------------------------------------------------------------------ */
#define USE_MAG
#define USE_MAG_HMC5883
#define USE_MAG_QMC5883
#define USE_MAG_LIS3MDL
#define USE_MAG_AK8963
#define USE_MAG_AK8975

/* ------------------------------------------------------------------------
 * ADC — voltage, current, RSSI, internal core temp
 * ------------------------------------------------------------------------ */
#define USE_ADC
#define USE_ADC_INTERNAL
#define ADC_VBAT_PIN            PC0
#define ADC_CURR_PIN            PA6
#define ADC_RSSI_PIN            PC1
#define ADC1_INSTANCE           ADC1
#define ADC3_INSTANCE           ADC3
#define ADC1_DMA_OPT            8
#define ADC3_DMA_OPT            9

#define BOARD_HAS_VOLTAGE_DIVIDER
#define ADC_VOLTAGE_REFERENCE_MV        3285
#define DEFAULT_VOLTAGE_METER_SCALE     176
#define DEFAULT_CURRENT_METER_SCALE     200
#define DEFAULT_VOLTAGE_METER_SOURCE    VOLTAGE_METER_ADC
#define DEFAULT_CURRENT_METER_SOURCE    CURRENT_METER_ADC

/* ------------------------------------------------------------------------
 * Motor outputs — 8 motors across two 4-in-1 ESC connectors.
 * SERVO1/2 share pins with UART6 (mutually exclusive at runtime).
 *
 * LED_STRIP intentionally OMITTED for v1 (PCB has hardware inverter on
 * PA3/TIM5_CH4 requiring TIMER_OUTPUT_INVERTED, which has no per-config
 * syntax in current master). Restore in v2 with platform-layer carve-out.
 * ------------------------------------------------------------------------ */
#define MOTOR1_PIN          PE11  // TIM1_CH2
#define MOTOR2_PIN          PE13  // TIM1_CH3
#define MOTOR3_PIN          PA15  // TIM2_CH1
#define MOTOR4_PIN          PA2   // TIM2_CH3
#define MOTOR5_PIN          PB5   // TIM3_CH2
#define MOTOR6_PIN          PB0   // TIM3_CH3
#define MOTOR7_PIN          PD13  // TIM4_CH2
#define MOTOR8_PIN          PD14  // TIM4_CH3
#define SERVO1_PIN          PC6   // TIM8_CH1, shared with UART6 TX
#define SERVO2_PIN          PC7   // TIM8_CH2, shared with UART6 RX

#define USE_TIMER_UP_CONFIG
#define TIMUP1_DMA_OPT      0
#define TIMUP2_DMA_OPT      0
#define TIMUP3_DMA_OPT      0
#define TIMUP4_DMA_OPT      0
#define TIMUP5_DMA_OPT      0
#define TIMUP8_DMA_OPT      2

#define TIMER_PIN_MAPPING \
    TIMER_PIN_MAP( 0, PE11, 1,  0) \
    TIMER_PIN_MAP( 1, PE13, 1,  1) \
    TIMER_PIN_MAP( 2, PA15, 1,  2) \
    TIMER_PIN_MAP( 3, PA2 , 1,  3) \
    TIMER_PIN_MAP( 4, PB5 , 1,  4) \
    TIMER_PIN_MAP( 5, PB0 , 2,  5) \
    TIMER_PIN_MAP( 6, PD13, 1,  6) \
    TIMER_PIN_MAP( 7, PD14, 1,  7) \
    TIMER_PIN_MAP( 8, PC6 , 2, -1) \
    TIMER_PIN_MAP( 9, PC7 , 2, -1)

/* ------------------------------------------------------------------------
 * Default features and serial routing
 * ------------------------------------------------------------------------ */
#define DEFAULT_FEATURES        (FEATURE_OSD)
#define SERIALRX_UART           SERIAL_PORT_USART7   // UART7 = HD VTX RC input
#define DEFAULT_RX_FEATURE      FEATURE_RX_SERIAL
#define SERIALRX_PROVIDER       SERIALRX_CRSF

// USART1 default = MSP DisplayPort for HD VTX (DJI / HDZero / Walksnail)
#define MSP_DISPLAYPORT_UART    SERIAL_PORT_USART1

/* ------------------------------------------------------------------------
 * IO ports — all GPIO banks fully pinned out on H750
 * ------------------------------------------------------------------------ */
#define TARGET_IO_PORTA     0xffff
#define TARGET_IO_PORTB     0xffff
#define TARGET_IO_PORTC     0xffff
#define TARGET_IO_PORTD     0xffff
#define TARGET_IO_PORTE     0xffff
#define TARGET_IO_PORTF     0xffff
#define TARGET_IO_PORTG     0xffff
