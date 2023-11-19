// WAVEGEN IP Library Registers
// Kalki and Prithvi

//-----------------------------------------------------------------------------
// Hardware Target
//-----------------------------------------------------------------------------

// Target Platform: DE1-SoC Board
// Target uC:       -
// System Clock:    -

// Hardware configuration:
// GPIO IP core connected to light-weight Avalon bus

//-----------------------------------------------------------------------------
// Device includes, defines, and assembler directives
//-----------------------------------------------------------------------------

#ifndef WAVEGENIP_REGS_H
#define WAVEGENIP_REGS_H

#define OFS_MODE             	0
#define OFS_RUN              	1
#define OFS_FREQA               2
#define OFS_FREQB       	    3
#define OFS_OFFSET     		    4
#define OFS_AMPLITUDE     	    5
#define OFS_DTYCYC    		    6
#define OFS_CYCLES 		        7

#define SPAN_IN_BYTES 32

#endif

