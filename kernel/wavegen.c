
//-----------------------------------------------------------------------------
// Hardware Target
//-----------------------------------------------------------------------------

// Target Platform: Xilinx XUP Blackboard

// Hardware configuration:
//
// AXI4-Lite interface:
//   Mapped to offset of 0
//
// WAVEGEN interface:
//   PMOD C is connected to SPI bus which writes to DAC and gives 2 waves in DAC A and DAC B
//-----------------------------------------------------------------------------

#include <stdlib.h>          // EXIT_ codes
#include <stdio.h>           // printf
#include <stdint.h>
#include <string.h>          // strcmp
#include "wavegen_ip.h"      // wavegen ip library
#include "wavegenIp_regs.h"

#define printd(format, ...) printf("[DEBUG][%d][%s][%s] " format "\n", __LINE__,__FILE__, __func__, ##__VA_ARGS__) // Debug print

#define MODE_DC         0
#define MODE_SINE       1
#define MODE_SAWTOOTH   2
#define MODE_TRIANGLE   3
#define MODE_SQUARE     4
#define MODE_ARB        5

typedef struct {
    char mode[10];  // "sine", "square", etc.
    char channel;    // 'A' or 'B'
    uint32_t mode_num;
    uint32_t freq;
    float amp;
    int32_t offset;
    uint32_t duty;
    uint32_t cycles;
    int isContinuous;
    int isDc;
} WaveGenArgs;

void parseArguments(int argc, char* argv[], WaveGenArgs* args) {
    // Initialize default values
    args->isContinuous = 1;
    args->isDc = 0;

    if (argc >= 4) {
        strncpy(args->mode, argv[1], sizeof(args->mode));
        args->channel = (argv[2][0] == 'A') ? 0 : 1;
        args->freq = atoi(argv[3]);
        args->amp = atof(argv[4]);

        if(strcmp(args->mode, "dc") == 0) args->mode_num = MODE_DC;
        if(strcmp(args->mode, "sine") == 0) args->mode_num = MODE_SINE;
        if(strcmp(args->mode, "saw") == 0) args->mode_num = MODE_SAWTOOTH;
        if(strcmp(args->mode, "tri") == 0) args->mode_num = MODE_TRIANGLE;
        if(strcmp(args->mode, "sq") == 0) args->mode_num = MODE_SQUARE;
        if(strcmp(args->mode, "arb") == 0) args->mode_num = MODE_ARB;


        if (argc >= 6) {
            args->offset = ((atof(argv[5])) * (1 << 14)) / 2.5;
        }

        if (argc >= 7) {
            args->duty = (atoi(argv[6]) * (1 << 14));
        }

        if (strcmp(args->mode, "dc") == 0) {
            args->isDc = 1;
        } else if (strcmp(args->mode, "cycles") == 0) {
            args->cycles = atoi(argv[5]);
            args->isContinuous = 0;
        }
    } else {
        // Handle invalid number of arguments
        printf("Invalid number of arguments\n");
        exit(EXIT_FAILURE);
    }
}

int main(int argc, char* argv[])
{
    /**
     * @brief Variables for different arguments
     * @todo give scale conversion factors to dc volts and offset and amplitude
     * @todo give default values all variables except mode
     * @todo Save the previous values in a file and read them out when run is received
     */
        waveGenOpen();
    WaveGenArgs args;

    parseArguments(argc, argv, &args);

    // Access parsed arguments
    printf("Mode: %s\n", args.mode);
    printf("Output: %c\n", args.channel);
    printf("Freq: %u\n", args.freq);
    printf("Amp: %f\n", args.amp);
    printf("Offset: %d\n", args.offset);
    printf("Duty: %u\n", args.duty);
    printf("Cycles: %u\n", args.cycles);
    printf("Continuous: %s\n", args.isContinuous ? "true" : "false");
    printf("DC: %s\n", args.isDc ? "true" : "false");



       // Set the values based on the parsed arguments
    if (strcmp(args.mode, "dc") == 0) {
        setOffset(args.channel, args.offset);
        setRun(args.channel, 1);
    } else if (strcmp(args.mode, "cycles") == 0) {
        if (strcmp(args.mode, "continuous") == 0) {
            setCycles(args.channel, 0);  // Set continuous
        } else {
            setCycles(args.channel, args.cycles);
        }
    } else {
        setChannelMode(args.channel, 1);  // Assuming mode setting is common for all waveform types
        setFrequency(args.channel, args.freq);
        setAmplitude(args.channel, (uint32_t)(args.amp * (1 << 14)));
        setOffset(args.channel, args.offset);
        setDutyCycle(args.channel, args.duty);
        setRun(args.channel, 1);
    }


    if (argc == 7)                                                      // “square OUT FREQ, AMP, [OFS, [DC]]”
    {
        printd();

        if (strcmp(argv[1], "square") == 0)                             // Get signal type
        {
            printd();
            if (atoi(argv[2]) == 0)                                     // Channel A
            {
                mode    = MODE_SQUARE;                                  // Set signal mode

                freq    = (atoi(argv[3]));
                amp     = ((atof(argv[4])) * (1 << 14));                // Convert to fixed point
                offset  = (((atof(argv[5])) * (1 << 14)) / -2.5);       // Convert to fixed point
                duty    = ((atoi(argv[6])) * (1 << 14));                // Convert to fixed point

                //ampA    = amp & MASK_A;                                 // Apply mask   [15:00] for Channel A
                offsetA = offset & MASK_A;                              // Apply mask   [15:00] for Channel A
                dutyA   = duty & MASK_A;                                // Apply mask   [15:00] for Channel A

                selectMode(mode);
                selectFreqA(freq);
                selectChannelAmplitudes(ampA);
                selectChannelOffsets(offsetA);
                selectChannelDutyCycles(dutyA);
            }

            else if (atoi(argv[2]) == 1)                                // Channel B
            {
                mode    = MODE_SQUARE << 3;                             // bits 5:3 for channel B

                freq    = atoi(argv[3]);
                amp     = ((atof(argv[4])) * (1 << 14));                // Convert to fixed point
                offset  = (((atof(argv[5])) * (1 << 14)) / -2.5);       // Convert to fixed point
                duty    = ((atoi(argv[6])) * (1 << 14));                // Convert to fixed point

                ////ampB    = ((amp << 16) & MASK_B);                       // Apply mask   [31:16] for Channel B
                offsetB = ((offset << 16) & MASK_B);                    // Apply mask   [31:16] for Channel B
                dutyB   = ((duty << 16) & MASK_B);                      // Apply mask   [31:16] for Channel B

                selectMode(mode);
                selectFreqB(freq);
                selectChannelAmplitudes(amp);
                selectChannelOffsets(offsetB);
                selectChannelDutyCycles(dutyB);
            }
            printd();
        }
    }

    else if (argc == 6)
    {
        printd();

        if (strcmp(argv[1], "sine") == 0)                               // “sine OUT, FREQ, AMP, [OFS]”
        {
            if (atoi(argv[2]) == 0)                                     // Channel A
            {
                mode    = MODE_SINE;                                    // Set signal mode

                freq    = atoi(argv[3]);
                amp     = ((atof(argv[4])) * (1 << 14));                // Convert to fixed point
                offset  = (((atof(argv[5])) * (1 << 14)) / -2.5);       // Convert to fixed point

                //ampA    = amp & MASK_A;                                 // Apply mask   [15:00] for Channel A
                offsetA = offset & MASK_A;                              // Apply mask   [15:00] for Channel A

                selectMode(mode);
                selectFreqA(freq);
                selectChannelAmplitudes(ampA);
                selectChannelOffsets(offsetA);
            }

            else if (atoi(argv[2]) == 1)                                // Channel B
            {
                mode    = MODE_SINE << 3;                               // bits 5:3 for channel B

                freq    = atoi(argv[3]);
                amp     = ((atof(argv[4])) * (1 << 14));                // Convert to fixed point
                offset  = (((atof(argv[5])) * (1 << 14)) / -2.5);       // Convert to fixed point

                //ampB    = amp & MASK_B;                                 // Apply mask   [31:16] for Channel B
                offsetB = offset & MASK_B;                              // Apply mask   [31:16] for Channel B

                selectMode(mode);
                selectFreqA(freq);
                selectChannelAmplitudes(ampB);
                selectChannelOffsets(offsetB);
            }
            printd();
        }

        else if (strcmp(argv[1], "sawtooth") == 0)                      // “sawtooth OUT, FREQ, AMP, [OFS]”
        {
            printd();
            if (atoi(argv[2]) == 0)                                     // Channel A
            {
                mode    = MODE_SAWTOOTH;                                // Set mode

                freq    = atoi(argv[3]);
                amp     = ((atof(argv[4])) * (1 << 14));                // Convert to fixed point
                offset  = (((atof(argv[5])) * (1 << 14)) / -2.5);       // Convert to fixed point

                //ampA    = amp & MASK_A;                                 // Apply mask   [15:00] for Channel A
                offsetA = offset & MASK_A;                              // Apply mask   [15:00] for Channel A

                selectMode(mode);
                selectFreqA(freq);
                selectChannelAmplitudes(ampA);
                selectChannelOffsets(offsetA);
            }

            else if (atoi(argv[2]) == 1)                                // Channel B
            {
                mode    = MODE_SAWTOOTH << 3;                           // bits 5:3 for channel B

                freq    = atoi(argv[3]);
                amp     = ((atof(argv[4])) * (1 << 14));                // Convert to fixed point
                offset  = (((atof(argv[5])) * (1 << 14)) / -2.5);       // Convert to fixed point

                //ampB    = amp & MASK_B;                                 // Apply mask   [31:16] for Channel B
                offsetB = offset & MASK_B;                              // Apply mask   [31:16] for Channel B

                selectMode(mode);
                selectFreqA(freq);
                selectChannelAmplitudes(ampB);
                selectChannelOffsets(offsetB);
            }
            printd();
        }

        else if (strcmp(argv[1], "triangle") == 0)                      // “triangle OUT, FREQ, AMP, [OFS]”
        {
            printd();
            if (atoi(argv[2]) == 0)                                     // Channel A
            {
                mode    = MODE_TRIANGLE;

                freq    = atoi(argv[3]);
                amp     = ((atof(argv[4])) * (1 << 14));                // Convert to fixed point
                offset  = (((atof(argv[5])) * (1 << 14)) / -2.5);       // Convert to fixed point

                //ampA    = amp & MASK_A;                                 // Apply mask   [15:00] for Channel A
                offsetA = offset & MASK_A;                              // Apply mask   [15:00] for Channel A

                selectMode(mode);
                selectFreqA(freq);
                selectChannelAmplitudes(ampA);
                selectChannelOffsets(offsetA);
            }
            else if (atoi(argv[2]) == 1)                                // Channel B
            {
                mode    = MODE_TRIANGLE << 3;                           // bits 5:3 for channel B

                freq    = atoi(argv[3]);
                amp     = ((atof(argv[4])) * (1 << 14));                // Convert to fixed point
                offset  = (((atof(argv[5])) * (1 << 14)) / -2.5);       // Convert to fixed point

                //ampB    = amp & MASK_B;                                 // Apply mask   [31:16] for Channel B
                offsetB = offset & MASK_B;                              // Apply mask   [31:16] for Channel B

                selectMode(mode);
                selectFreqA(freq);
                selectChannelAmplitudes(ampB);
                selectChannelOffsets(offsetB);
            }
            printd();
        }

        else if (strcmp(argv[1], "square") == 0)                        // “square OUT FREQ, AMP, [OFS]”
        {
            printd();
            if (atoi(argv[2]) == 0)                                     // Channel A
            {
                mode = MODE_SQUARE;

                freq    = atoi(argv[3]);
                amp     = ((atof(argv[4])) * (1 << 14));                // Convert to fixed point
                offset  = (((atof(argv[5])) * (1 << 14)) / -2.5);       // Convert to fixed point

                //ampA    = amp & MASK_A;                                 // Apply mask   [15:00] for Channel A
                offsetA = offset & MASK_A;                              // Apply mask   [15:00] for Channel A

                selectMode(mode);
                selectFreqA(freq);
                selectChannelAmplitudes(ampA);
                selectChannelOffsets(offsetA);
            }

            else if (atoi(argv[2]) == 1)                                // Channel B
            {
                mode    = MODE_SQUARE << 3;                             // bits 5:3 for channel B

                freq    = atoi(argv[3]);
                amp     = ((atof(argv[4])) * (1 << 14));                // Convert to fixed point
                offset  = (((atof(argv[5])) * (1 << 14)) / -2.5);       // Convert to fixed point

                //ampB    = amp & MASK_B;                                 // Apply mask   [31:16] for Channel B
                offsetB = offset & MASK_B;                              // Apply mask   [31:16] for Channel B

                selectMode(mode);
                selectFreqA(freq);
                selectChannelAmplitudes(ampB);
                selectChannelOffsets(offsetB);
            }
            printd();
        }
    }

    /**
     * @brief OFS argument are DEFAULT
     *
     */
    else if (argc == 5)
    {
        printd();

        if (strcmp(argv[1], "sine") == 0)                               // “sine OUT, FREQ, AMP, [OFS]”
        {
            if (atoi(argv[2]) == 0)                                     // Channel A
            {
                mode    = MODE_SINE;                               // bits 5:3 for channel B
                freq    = atoi(argv[3]);
                amp     = ((atof(argv[4])) * (1 << 14));                // Convert to fixed point

                ampA     = ((uint16_t)amp) & MASK_A;                                 // Apply mask   [15:00] for Channel A

                selectMode(mode);
                selectFreqA(freq);
                selectChannelAmplitudes(ampA);
            }

            else if (atoi(argv[2]) == 1)                                // Channel B
            {
                mode    = MODE_SINE << 3;                               // bits 5:3 for channel B

                freq    = atoi(argv[3]);
                amp     = ((atof(argv[4])) * (1 << 14));                // Convert to fixed point
                ampB    = ((uint16_t)amp) & MASK_B;                                 // Apply mask   [31:16] for Channel B

                selectMode(mode);
                selectFreqA(freq);
                selectChannelAmplitudes(ampB);
            }
            printd();
        }

        else if (strcmp(argv[1], "sawtooth") == 0)                      // “sawtooth OUT, FREQ, AMP, [OFS]”
        {
            printd();
            if (atoi(argv[2]) == 0)                                     // Channel A
            {
                mode    = MODE_SAWTOOTH;
                freq    = atoi(argv[3]);
                amp     = ((atof(argv[4])) * (1 << 14));                // Convert to fixed point
                //ampA    = amp & MASK_A;                                 // Apply mask   [15:00] for Channel A

                selectMode(mode);
                selectFreqA(freq);
                selectChannelAmplitudes(ampA);
            }

            else if (atoi(argv[2]) == 1)                                // Channel B
            {
                mode    = MODE_SAWTOOTH << 3;                           // bits 5:3 for channel B
                freq    = atoi(argv[3]);
                amp     = ((atof(argv[4])) * (1 << 14));                // Convert to fixed point
                //ampB    = amp & MASK_B;                                 // Apply mask   [31:16] for Channel B

                selectMode(mode);
                selectFreqA(freq);
                selectChannelAmplitudes(ampB);
            }
            printd();
        }

        else if (strcmp(argv[1], "triangle") == 0)                      // “triangle OUT, FREQ, AMP, [OFS]”
        {
            printd();
            if (atoi(argv[2]) == 0)                                     // Channel A
            {
                mode    = MODE_TRIANGLE;

                freq    = atoi(argv[3]);
                amp     = ((atof(argv[4])) * (1 << 14));                // Convert to fixed point
                //ampA    = amp & MASK_A;                                 // Apply mask   [15:00] for Channel A

                selectMode(mode);
                selectFreqA(freq);
                selectChannelAmplitudes(ampA);
            }

            else if(atoi(argv[2]) == 1)
            {
                mode    = MODE_TRIANGLE << 3;                           // bits 5:3 for channel B
                freq    = atoi(argv[3]);
                amp     = ((atof(argv[4])) * (1 << 14));                // Convert to fixed point
                //ampB    = amp & MASK_B;                                 // Apply mask   [31:16] for Channel B

                selectMode(mode);
                selectFreqA(freq);
                selectChannelAmplitudes(ampB);
            }
            printd();
        }
    }

    /**
     * @brief DC mode and CYCLES setting
     *
     */
    else if (argc == 4)
    {

        if (strcmp(argv[1], "dc") == 0)                                 //“dc OUT, OFS”
        {
            if (atoi(argv[2]) == 0)                                     // Channel A
            {
                mode    = MODE_DC;
                offset  = (((atof(argv[3])) * (1 << 14)) / 2.5);       // Convert to fixed point
                offsetA = offset & MASK_A;                                 // Apply mask   [15:00] for Channel A

                selectMode(mode);
                selectChannelOffsets(offset);
            }

            else if (atoi(argv[2]) == 1)                                // Channel B
            {
                mode    = MODE_DC << 3;                                 // bits 5:3 for channel B
                offset  = (((atof(argv[3])) * (1 << 14)) / 2.5);       // Convert to fixed point
                offsetB = (offset << 16) & MASK_B;                             // Apply mask   [15:00] for Channel A

                printd("--->%d", offsetB);

                selectMode(mode);
                selectChannelOffsets(offsetB);
            }
        }

        if (strcmp(argv[1], "cycles") == 0)                             //“cycles OUT, N”
        {
            if (atoi(argv[2]) == 0)                                     // Channel A
            {
                cycles  = atoi(argv[3]);
                cyclesA = cycles & MASK_A;

                selectChannelCycles(cyclesA);
            }

            else if (atoi(argv[2]) == 1)                                // Channel B
            {
                cycles  = atoi(argv[3]);
                cyclesB = (cycles<<16) & MASK_B;

                selectChannelCycles(cyclesB);
            }
        }
        printd();
    }

    else if (argc == 3)
    {
        printd();

        if (strcmp(argv[1], "cycles") == 0)                             // “cycles continuous”
        {
            if (strcmp(argv[1], "cont") == 0)
            {
                cycles = 0;
                selectChannelCycles(cycles);
            }
        }
        printd();
    }

    else  if (argc == 2)
    {
        printd();

        if (strcmp(argv[1], "run") == 0)                                // “cycles continuous”
        {
            uint32_t run = getRegValue(OFS_RUN);
            if(run & 0x1)
            {
                // Channel A is running
                //Get the previous values and set them again
                /**
                 * @todo
                 *
                 */
            }
        }

        else if (strcmp(argv[1], "stop") == 0)                          // Set a DC voltage
        {
            selectMode(MODE_DC | (MODE_DC << 3));
            selectChannelOffsets(0);
        }
        else if (strcmp(argv[1], "status") == 0)                        // Set a DC voltage
        {
            uint32_t regVal;
            uint8_t i;
            for (i = 0; i < 8; i++)
            {
                regVal = getRegValue(i);
                printf(" Register offset %d : RegValue : %hx \n", i, regVal);
            }
        }
        printd();
    }

    else if (argc == 2 && (strcmp(argv[1], "-h") == 0 || strcmp(argv[1], "--help") == 0))
    {
        printd();
        printf("  usage:\n");
        printf("  dc OUT, OFS           make the pin a push-pull output\n");
        printf("  cycles OUT, N            make the pin an open drain output\n");
        printf("  \n");
        printf("  sine OUT, FREQ, AMP, [OFS]            make the pin an input\n");
        printf("  sawtooth OUT, FREQ, AMP, [OFS]          set the pin high\n");
        printf("  triangle OUT, FREQ, AMP, [OFS]           set the pin low\n");
        printf("  square OUT FREQ, AMP, [OFS, [DC]]        get the pin status\n");
        printf("  \n");
        printf("  run \n");
        printf("  stop \n");
        printf("  status \n");
        printf("  \n");
        printd();
    }
    else
    {
        printf("  command not understood\n");
        printd();
    }
    return EXIT_SUCCESS;
}
