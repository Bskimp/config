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
 * BrainFPV RADIX 2 (HD-VTX-only variant) — Betaflight master.
 *
 * Sibling target to RADIX2HD. Same MCU (STM32H750), same vendor bootloader
 * family, but a different bootloader binary that uses the SPRACING-style
 * RAM-copy EXST flow rather than HD's XIP-from-QSPI. Most differences from
 * the HD config are pin-map; the build-system delta is in mk/config.mk +
 * STM32H7.mk + brainfpv_pack.mk (linker selection and packer --dev arg
 * keyed on BOARD_NAME).
 *
 * Scope: HD VTX users (DJI O3 / Walksnail / HDZero) on MSP DisplayPort.
 * Analog graphical OSD subsystem intentionally not ported — see
 * RADIX2_porting_roadmap.md.
 *
 * Pin map confirmed against:
 *   - BrainFPV/betaflight @ 4.5.x BRAIN-2 src/main/target/RADIX2/target.h
 *   - Live board CLI dump (2026-05-02), `Radix 2 dump and extras.txt`
 */

#pragma once

#define FC_TARGET_MCU       STM32H750

#define BOARD_NAME          RADIX2
#define MANUFACTURER_ID     BRFP

#define TARGET_BOARD_IDENTIFIER "RDX2"
#define USBD_PRODUCT_STRING     "BrainFPV RADIX 2"

// 16 MHz external crystal. Same MCU/board family as HD.
#define SYSTEM_HSE_MHZ      16

/* ------------------------------------------------------------------------
 * Firmware layout — RAM-copy EXST flow
 *
 * Firmware lives at 0x24010000 (CODE_RAM) at runtime. The BrainFPV
 * bootloader copies the packed image from its staging region to AXI SRAM
 * before jumping. This is master's stock H750 EXST flow and uses the
 * existing stm32_ram_h750_exst.ld unchanged.
 *
 * The packer (radix2.json, needs_header=true) wraps the binary with its
 * own envelope, so we do NOT define BRAINFPV_BL_HEADER_MAGIC — there is no
 * .bl_header_section to populate. The HD-style firmware-image magic is
 * supplied by the packer header on this device.
 * ------------------------------------------------------------------------ */
#define FC_VMA_ADDRESS              0x24010000
#define CONFIG_IN_EXTERNAL_FLASH
#define USE_FIRMWARE_PARTITION
#define EEPROM_SIZE                 8192

/* ------------------------------------------------------------------------
 * Bootloader handshake — only the persistent.c BKP_DR6 dual-write
 *
 * Same MCU-level mechanism as HD: when systemResetToBootloader() writes
 * RESET_BOOTLOADER_REQUEST_ROM to PERSISTENT_OBJECT_RESET_REASON, the
 * USE_BRAINFPV_BL #ifdef block in src/platform/STM32/persistent.c
 * dual-writes to PERSISTENT_OBJECT_BRAINFPV_BL (BKP_DR6). The RADIX 2
 * bootloader reads that slot on boot to decide DFU-vs-jump.
 *
 * NB: BRAINFPV_BL_HEADER_MAGIC intentionally NOT defined — see firmware
 * layout block above. Packer's needs_header=true covers that role.
 * ------------------------------------------------------------------------ */
#define USE_BRAINFPV_BL

/* ------------------------------------------------------------------------
 * Status LEDs (direct STM32 pins, NOT via FPGA)
 * ------------------------------------------------------------------------ */
#define LED0_PIN            PE6
#define LED0_INVERTED
#define LED1_PIN            PE7
#define LED1_INVERTED

/* ------------------------------------------------------------------------
 * Beeper (direct pin, NOT via FPGA)
 * ------------------------------------------------------------------------ */
#define USE_BEEPER
#define BEEPER_PIN          PD14
#define BEEPER_INVERTED

/* ------------------------------------------------------------------------
 * PINIO — VTX 9V switch + video input mux
 * ------------------------------------------------------------------------ */
#define USE_PINIO
#define USE_PINIOBOX
#define PINIO1_PIN          PD15    // 9V VTX pit switch
#define PINIO1_BOX          40      // BOXPIT permanent ID
#define PINIO2_PIN          PC15    // video input mux (irrelevant for HD VTX users)

/* ------------------------------------------------------------------------
 * UARTs — pin map from live board dump
 *
 * No UART7 on this board (HD board has it for HD VTX RC input; RADIX 2
 * uses USART3 for RC input by default per the legacy target).
 * ------------------------------------------------------------------------ */
#define UART1_TX_PIN        PB6
#define UART1_RX_PIN        PB15
#define UART2_TX_PIN        PA2     // shared with SERVO1 (TIM15_CH1)
#define UART2_RX_PIN        PA3     // shared with SERVO2 (TIM15_CH2)
#define UART3_TX_PIN        PD8
#define UART3_RX_PIN        PB11
#define UART4_TX_PIN        PD1
#define UART4_RX_PIN        PD0
#define UART5_TX_PIN        PB13
#define UART5_RX_PIN        PB12
#define UART6_TX_PIN        PC6
#define UART6_RX_PIN        PC7

#define USE_VCP
#define USE_USB_DETECT
#define USB_DETECT_PIN      PA9

/* ------------------------------------------------------------------------
 * SPI buses
 *   SPI1 -> blackbox flash (W25Q128 on standard SPI, NOT QSPI like HD)
 *   SPI2 -> BMI270 IMU
 *   SPI3 -> Lattice FPGA (held in reset by configTargetPreInit() in config.c;
 *           pins NOT exposed at the BF SPI driver layer)
 * ------------------------------------------------------------------------ */
#define USE_SPI

#define USE_SPI_DEVICE_1
#define SPI1_SCK_PIN        PA5
#define SPI1_SDI_PIN        PB4
#define SPI1_SDO_PIN        PD7

#define USE_SPI_DEVICE_2
#define SPI2_SCK_PIN        PD3
#define SPI2_SDI_PIN        PC2
#define SPI2_SDO_PIN        PC1
#define SPI2_NSS_PIN        PE15    // == GYRO_1_CS_PIN

/* ------------------------------------------------------------------------
 * I2C — external compass / airspeed / DPS310 baro
 * ------------------------------------------------------------------------ */
#define USE_I2C
#define USE_I2C_DEVICE_1
#define I2C1_SCL_PIN        PB8
#define I2C1_SDA_PIN        PB7
#define I2C_DEVICE          I2CDEV_1
#define MAG_I2C_INSTANCE    I2CDEV_1

/* ------------------------------------------------------------------------
 * Blackbox flash — W25Q128FV on SPI1 (NOT QSPI)
 *
 * Live board reports JEDEC ID 0x00016018 (Winbond W25Q128FV). Same chip
 * as the HD board, just on a different bus. USE_FLASH_QUADSPI / USE_QUADSPI
 * deliberately omitted on this target.
 * ------------------------------------------------------------------------ */
#define USE_FLASH
#define USE_FLASH_W25Q128FV
#define USE_FLASHFS

#define FLASH_CS_PIN            PE14
#define FLASH_SPI_INSTANCE      SPI1

#define DEFAULT_BLACKBOX_DEVICE BLACKBOX_DEVICE_FLASH

/* ------------------------------------------------------------------------
 * Gyro / accel — BMI270 on SPI2
 * ------------------------------------------------------------------------ */
#define USE_EXTI
#define USE_GYRO
#define USE_ACC
#define USE_GYRO_EXTI
#define USE_SPI_GYRO
#define USE_ACCGYRO_BMI270

#define GYRO_1_EXTI_PIN         PE4
#define GYRO_1_CS_PIN           PE15
#define GYRO_1_SPI_INSTANCE     SPI2
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
 * ADC
 *   VBAT  -> PA6  (ADC1)
 *   CURR  -> PB0  (ADC1 / ADC2)
 *   RSSI  -> PC0  (ADC1)
 *
 * ADC2 is genuinely unused on this build (was the analog-OSD sync threshold
 * sampler in the BrainFPV fork; no analog OSD = no ADC2 customer).
 * ------------------------------------------------------------------------ */
#define USE_ADC
#define USE_ADC_INTERNAL
#define ADC_VBAT_PIN            PA6
#define ADC_CURR_PIN            PB0
#define ADC_RSSI_PIN            PC0
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
 * Camera control + RX PPM legacy pins
 * ------------------------------------------------------------------------ */
#define USE_CAMERA_CONTROL
#define CAMERA_CONTROL_PIN      PA7

/* ------------------------------------------------------------------------
 * Motors + servos
 *   M1 PA0  TIM2_CH1
 *   M2 PB5  TIM3_CH2
 *   M3 PD12 TIM4_CH1
 *   M4 PD13 TIM4_CH2
 *   M5 PC9  TIM8_CH4
 *   M6 PC8  TIM8_CH3
 *   M7 PE13 TIM1_CH3
 *   M8 PE11 TIM1_CH2
 *   S1 PA2  TIM15_CH1 (shared with UART2 TX)
 *   S2 PA3  TIM15_CH2 (shared with UART2 RX)
 *
 * No LED strip in v1 — see roadmap. WS2811 routing on RADIX 2 schematic
 * needs schematic check; pin reserved for v2.
 * ------------------------------------------------------------------------ */
#define MOTOR1_PIN          PA0
#define MOTOR2_PIN          PB5
#define MOTOR3_PIN          PD12
#define MOTOR4_PIN          PD13
#define MOTOR5_PIN          PC9
#define MOTOR6_PIN          PC8
#define MOTOR7_PIN          PE13
#define MOTOR8_PIN          PE11
#define SERVO1_PIN          PA2
#define SERVO2_PIN          PA3

#define USE_TIMER_UP_CONFIG
#define TIMUP1_DMA_OPT      0
#define TIMUP2_DMA_OPT      0
#define TIMUP3_DMA_OPT      0
#define TIMUP4_DMA_OPT      0
#define TIMUP8_DMA_OPT      2
#define TIMUP15_DMA_OPT     0

#define TIMER_PIN_MAPPING \
    TIMER_PIN_MAP( 0, PA0 , 1,  0) \
    TIMER_PIN_MAP( 1, PB5 , 1,  1) \
    TIMER_PIN_MAP( 2, PD12, 1,  2) \
    TIMER_PIN_MAP( 3, PD13, 1,  3) \
    TIMER_PIN_MAP( 4, PC9 , 1,  4) \
    TIMER_PIN_MAP( 5, PC8 , 1,  5) \
    TIMER_PIN_MAP( 6, PE13, 1,  6) \
    TIMER_PIN_MAP( 7, PE11, 1,  7) \
    TIMER_PIN_MAP( 8, PA2 , 2, -1) \
    TIMER_PIN_MAP( 9, PA3 , 2, -1)

/* ------------------------------------------------------------------------
 * Default features and serial routing
 *
 * RC input via USART3 (CRSF). HD VTX MSP DisplayPort on USART1 by default.
 * ------------------------------------------------------------------------ */
#define DEFAULT_FEATURES        (FEATURE_OSD)
#define SERIALRX_UART           SERIAL_PORT_USART3
#define DEFAULT_RX_FEATURE      FEATURE_RX_SERIAL
#define SERIALRX_PROVIDER       SERIALRX_CRSF

#define MSP_DISPLAYPORT_UART    SERIAL_PORT_USART1

/* ------------------------------------------------------------------------
 * IO ports
 * ------------------------------------------------------------------------ */
#define TARGET_IO_PORTA     0xffff
#define TARGET_IO_PORTB     0xffff
#define TARGET_IO_PORTC     0xffff
#define TARGET_IO_PORTD     0xffff
#define TARGET_IO_PORTE     0xffff
#define TARGET_IO_PORTF     0xffff
#define TARGET_IO_PORTG     0xffff
