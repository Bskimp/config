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
 * BrainFPV RADIX 2 HD — bootloader header struct.
 *
 * The BrainFPV bootloader reads BRAINFPV_BL_HEADER from flash before
 * jumping to firmware. Both fields must be present at the firmware load
 * address (0x90400000) inside a `.bl_header_section` placed by the
 * custom linker script (stm32_h750_brainfpv_xip.ld).
 *
 * Without this struct in the image the bootloader will refuse to jump.
 *
 * Cloned verbatim (modulo gating) from BrainFPV/betaflight @ 4.5.1-BRAIN-2,
 * file lib/main/brainfpv/brainfpv_system.c.
 */

#include <stdint.h>
#include "platform.h"

/* The BL header struct is only needed when the firmware itself must carry the
 * bootloader-validation magic (RADIX 2 HD: packer needs_header=false, struct
 * lives in the firmware image). RADIX 2 has needs_header=true — the packer
 * supplies its own envelope and the firmware doesn't need this struct. So we
 * gate on the magic constant being defined rather than on USE_BRAINFPV_BL,
 * which is also set by RADIX 2 (for the persistent.c BKP_DR6 carve-out). */
#ifdef BRAINFPV_BL_HEADER_MAGIC

typedef struct __attribute__((packed)) {
    uint32_t target_magic;
    uint32_t isr_vector_base;
} BrainFPVBlHeader_t;

/* Provided by stm32_h750_common.ld inside the .isr_vector section:
 *     PROVIDE (isr_vector_table_base = .);
 * This symbol resolves to the actual vector table address — which sits AFTER
 * our .bl_header_section pad (typically 0x90400000 + 0x200 = 0x90400200).
 * Using the symbol rather than a hardcoded offset keeps things robust against
 * future changes to the .bl_header_section size or alignment. */
extern char isr_vector_table_base[];

const BrainFPVBlHeader_t __attribute__((section(".bl_header_section"))) __attribute__((used)) BRAINFPV_BL_HEADER = {
    .target_magic    = BRAINFPV_BL_HEADER_MAGIC,
    .isr_vector_base = (uint32_t)(uintptr_t)isr_vector_table_base,
};

#endif /* BRAINFPV_BL_HEADER_MAGIC */
