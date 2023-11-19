//-----------------------------------------------------------------------------
// Hardware Target
//-----------------------------------------------------------------------------

// Target Platform: Xilinx XUP Blackboard

// AXI4-Lite interface:
//   Mapped to offset of 0
//
//
// WAVEGEN interface:
//   PMOD C is connected to SPI bus which writes to DAC and gives 2 waves in DAC A and DAC B

//-----------------------------------------------------------------------------

#include <stdint.h>          // C99 integer types -- uint32_t
#include <stdbool.h>         // bool
#include <fcntl.h>           // open
#include <sys/mman.h>        // mmap
#include <unistd.h>          // close
#include "../address_map.h"  // address map
#include "wavegen_ip.h"     // wavegen functions
#include "wavegenIp_regs.h" // wavegen registers

#include <stdio.h>
#define printd(format, ...) printf("[DEBUG][%d][%s][%s] " format "\n", __LINE__, __FILE__, __func__, ##__VA_ARGS__) // Debug print

//-----------------------------------------------------------------------------
// Global variables
//-----------------------------------------------------------------------------

#define CHANNEL_A_MODE_MASK 0x07    // 0b00000111
#define CHANNEL_B_MODE_MASK 0x38    // 0b00111000
#define FREQ_MASK 0xFFFFFFFF  // 32-bit mask


uint32_t *base = NULL;

//-----------------------------------------------------------------------------
// Subroutines
//-----------------------------------------------------------------------------

bool waveGenOpen()
{
    // Open /dev/mem
    int file = open("/dev/mem", O_RDWR | O_SYNC);
    bool bOK = (file >= 0);
    if (bOK)
    {
        // Create a map from the physical memory location of
        // /dev/mem at an offset to LW avalon interface
        // with an aperture of SPAN_IN_BYTES bytes
        // to any location in the virtual 32-bit memory space of the process
        base = mmap(NULL, SPAN_IN_BYTES, PROT_READ | PROT_WRITE, MAP_SHARED,
                    file, AXI4_LITE_BASE + WAVEGEN_IP_OFFSET);
        bOK = (base != MAP_FAILED);

        // Close /dev/mem
        close(file);
    }
    printd();
    return bOK;
}

void setChannelMode(uint32_t channel, uint32_t mode) 
{
    uint32_t regValue = *(base + OFS_MODE);    // Read the current register value

    if (channel == 0) {    //READ... Clear the mode bits for the specified channel
        regValue &= ~CHANNEL_A_MODE_MASK;
    } else if (channel == 1) {
        regValue &= ~CHANNEL_B_MODE_MASK;
    }

    if (channel == 0) {    //MODIFY Set the new mode bits for the specified channel
        regValue |= (mode & CHANNEL_A_MODE_MASK);
    } else if (channel == 1) {
        regValue |= ((mode << 3) & CHANNEL_B_MODE_MASK);
    }

    *(base + OFS_MODE) = regValue;    // WRITE the modified value back to the register
}

void setFrequency(uint32_t channel, uint32_t frequency)
 {
    uint32_t offset = (channel == 0) ? OFS_FREQA : OFS_FREQB;    // Determine the register offset based on the channel
    uint32_t regValue = *(base + offset);    // Read the current register value
    regValue &= ~0xFFFFFFFF;      // Clear the existing frequency bits
    regValue |= frequency;    // Set the new frequency bits

    *(base + offset) = regValue;    // Write the modified value back to the register
}

void setDutyCycle(uint32_t channel, uint32_t dutyCycle) {
    uint32_t offset = OFS_DUTYCYC;    // Determine the offset based on the channel

    if (channel == 1) {    // If Channel B, shift the dutyCycle to the upper 16 bits
        dutyCycle <<= 16;
    }

    uint32_t regValue = *(base + offset);    // Read the current register value
    if (channel == 0) {    // Clear the existing duty cycle bits
        regValue &= ~0x0000FFFF;  // Clear bits 15:0 for Channel A
    } else {
        regValue &= 0x0000FFFF;   // Clear bits 31:16 for Channel B
    }

    regValue |= dutyCycle;    // Set the new duty cycle bits
    *(base + offset) = regValue;    // Write the modified value back to the register

}

void setAmplitude(uint32_t channel, uint32_t amplitude) {
    uint32_t offset = OFS_AMPLITUDE;    // Determine the offset based on the channel

    if (channel == 1) {    // If Channel B, shift the amplitude to the upper 16 bits
        amplitude <<= 16;
    }

    uint32_t regValue = *(base + offset);    // Read the current register value
    if (channel == 0) {    // Clear the existing amplitude bits
        regValue &= ~0x0000FFFF;  // Clear bits 15:0 for Channel A
    } else {
        regValue &= 0x0000FFFF;   // Clear bits 31:16 for Channel B
    }

    regValue |= amplitude;    // Set the new amplitude bits
    *(base + offset) = regValue;    // Write the modified value back to the register
}

void setOffset(uint32_t* base, uint32_t channel, int32_t offset) {
    uint32_t offsetRegister = OFS_OFFSET;    // Determine the offset based on the channel

    if (channel == 1) {    // If Channel B, shift the offset to bits 31:16
        offset <<= 16;
    }

    uint32_t regValue = *(base + offsetRegister);    // Read the current register value

    if (channel == 0) {    // Clear the existing offset bits for the specified channel
        regValue &= ~0x0000FFFF;  // Clear bits 15:0 for Channel A
    } else {
        regValue &= 0xFFFF0000;   // Clear bits 31:16 for Channel B
    }

    regValue |= (uint32_t)offset;    // Set the new offset bits for the specified channel
    *(base + offsetRegister) = regValue;    // Write the modified value back to the register

}

void setCycles(uint32_t channel, uint32_t cycles) {
    uint32_t offset = OFS_CYCLES;    // Determine the offset based on the channel

    if (channel == 1) {    // If Channel B, shift the cycles to the upper 16 bits
        cycles <<= 16;
    }

    uint32_t regValue = *(base + offset);    // Read the current register value

    if (channel == 0) {    // Clear the existing cycles bits
        regValue &= ~0x0000FFFF;  // Clear bits 15:0 for Channel A
    } else {
        regValue &= 0x0000FFFF;   // Clear bits 31:16 for Channel B
    }

    regValue |= cycles;    // Set the new cycles bits
    *(base + offset) = regValue;    // Write the modified value back to the register

}
void setRun(uint32_t channel, uint32_t run) {
    uint32_t offset = OFS_RUN;    // Determine the offset based on the channel

    if (channel == 1) {    // If Channel B, shift the run bit to bit 1
        run <<= 1;
    }

    uint32_t regValue = *(base + offset);    // Read the current register value

    regValue &= ~(1 << channel);    // Clear the existing run bit for the specified channel

    regValue |= run;    // Set the new run bit for the specified channel

    *(base + offset) = regValue;    // Write the modified value back to the register
}
