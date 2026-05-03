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
 * BrainFPV RADIX 2 — runtime init customization.
 *
 * Single responsibility: hold the on-board Lattice FPGA in reset before
 * peripherals come up. The FPGA on this board was used by BrainFPV for the
 * analog gOSD subsystem (composite-video generation, DAC sync, camera/VTX
 * mux, etc.). On power-up it may drive CSYNC and video-out into the VTX
 * and camera signal paths. We don't drive it in this firmware (the analog
 * OSD subsystem is intentionally not ported), so we keep it parked in
 * reset. PC4 is the FPGA's RESET line per BrainFPV/RADIX2/target.h.
 *
 * No BL_HEADER struct — RADIX 2's packer (radix2.json) has
 * needs_header=true and supplies the firmware-image envelope itself.
 */

#include <stdint.h>
#include "platform.h"
#include "drivers/io.h"

#define FPGA_RESET_IO_TAG       IO_TAG(PC4)

void configTargetPreInit(void)
{
    IO_t fpgaResetPin = IOGetByTag(FPGA_RESET_IO_TAG);
    IOInit(fpgaResetPin, OWNER_SYSTEM, 0);
    IOConfigGPIO(fpgaResetPin, IOCFG_OUT_PP);
    IOLo(fpgaResetPin);   // hold FPGA in reset
}
