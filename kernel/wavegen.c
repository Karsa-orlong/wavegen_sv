
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

#include <stdlib.h> // EXIT_ codes
#include <stdio.h>  // printf
#include <stdint.h>
#include <string.h>     // strcmp
#include "wavegen_ip.h" // wavegen ip library
#include "wavegenIp_regs.h"

#define printd(format, ...) printf("[DEBUG][%d][%s][%s] " format "\n", __LINE__, __FILE__, __func__, ##__VA_ARGS__) // Debug print

#define MODE_DC 0
#define MODE_SINE 1
#define MODE_SAWTOOTH 2
#define MODE_TRIANGLE 3
#define MODE_SQUARE 4
#define MODE_ARB 5

typedef struct
{
    volatile char mode[10];    // "sine", "square", etc.
    volatile uint32_t channel; // 'A' or 'B'
    volatile uint32_t mode_num;
    volatile uint32_t freq;
    volatile float amp;
    volatile float offset;
    volatile float duty;
    volatile uint32_t cycles;
    volatile int isDc;
} WaveGenArgs;

typedef struct
{
    volatile uint32_t amp_fp;
    volatile int32_t offset_fp;
    volatile uint32_t duty_fp;
} wavegen_fpArgs;
void parseArguments(int argc, char *argv[], WaveGenArgs *args, wavegen_fpArgs *fpArgs)
{
    // Initialize default values
    args->isDc = 0;
    printd();
    strncpy((char *)args->mode, argv[1], sizeof(args->mode));

    if (argc > 4)
    {
        args->channel = (argv[2][0] == 'A') ? 0 : 1;
        args->freq = atoi(argv[3]);
        args->amp = atof(argv[4]);
        fpArgs->amp_fp = (args->amp) * (1 << 14);

        if      (strcmp((char *)args->mode, "sine") == 0)   args->mode_num = MODE_SINE;
        else if (strcmp((char *)args->mode, "saw") == 0)    args->mode_num = MODE_SAWTOOTH;
        else if (strcmp((char *)args->mode, "tri") == 0)    args->mode_num = MODE_TRIANGLE;
        else if (strcmp((char *)args->mode, "sq") == 0)     args->mode_num = MODE_SQUARE;
        else if (strcmp((char *)args->mode, "arb") == 0)    args->mode_num = MODE_ARB;

        if (argc >= 6)
        {
            args->offset = atof(argv[5]);
            fpArgs->offset_fp = (args->offset) * (1 << 14) / 2.5;
        }

        if (argc >= 7)
        {
            args->duty = atof(argv[6]);
            fpArgs->duty_fp = (args->duty) * (1 << 14);
        }
    }

    else if (argc == 4)
    {
        args->channel = (argv[2][0] == 'A') ? 0 : 1;

        if (!strcmp((char *)args->mode, "dc"))
        {
            args->mode_num = MODE_DC;
            args->isDc = 1;
            args->offset = atof(argv[3]);
            fpArgs->offset_fp = (args->offset) * (1 << 14) / 2.5;
            printd();
        }

        else if (!strcmp((char *)args->mode, "cycles"))
        {
            args->cycles = atoi(argv[3]);
        }

    }
    else
    {
        // Handle invalid number of arguments
        printf("  command not understood\n");
    }
    printd();
}

int main(int argc, char *argv[])
{
    /**
     * @brief Variables for different arguments
     * @todo give scale conversion factors to dc volts and offset and amplitude
     * @todo give default values all variables except mode
     * @todo Save the previous values in a file and read them out when run is received
     */
    printd();

    waveGenOpen();

    WaveGenArgs args = {0};
    wavegen_fpArgs fpArgs = {0};

    parseArguments(argc, argv, &args, &fpArgs);

    // Access parsed arguments
    printf("Mode:\t %s \nModN:\t%d\n",    args.mode, args.mode_num);
    printf("Chan:\t %d \n",               args.channel);
    printf("Freq:\t %u \n",               args.freq);
    printf("Ampl:\t %f \t %d\n",          args.amp, fpArgs.amp_fp);
    printf("Ofst:\t %f \t %d\n",          args.offset, fpArgs.offset_fp);
    printf("Duty:\t %f \t %d\n",          args.duty, fpArgs.duty_fp);
    printf("Cycl:\t %u \n",               args.cycles);

    if (argc == 2 && (strcmp(argv[1], "-h") == 0 || strcmp(argv[1], "--help") == 0))
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
    }

    // Set the values based on the parsed arguments
    if (strcmp(argv[1], "dc") == 0)
    {
        setChannelMode(args.channel, args.mode_num); // Assuming mode setting is common for all waveform types
        setOffset(args.channel, fpArgs.offset_fp);
        setRun(args.channel, 1);
    }
    else if (strcmp(argv[1], "cycles") == 0)
    {
        setCycles(args.channel, args.cycles); // Set continuous
    }

    else
    {
        setChannelMode(args.channel, args.mode_num); // Assuming mode setting is common for all waveform types
        setFrequency(args.channel, args.freq);
        setAmplitude(args.channel, fpArgs.amp_fp);
        setOffset(args.channel, fpArgs.offset_fp);
        setDutyCycle(args.channel, fpArgs.duty_fp);
        setRun(args.channel, 1);
    }

    if (strcmp(argv[1], "stop") == 0)
    {
        uint8_t i;
        for (i = 0; i < 2; i++)
        {
            setChannelMode(i, 0); // Assuming mode setting is common for all waveform types
            setFrequency(i, 0);
            setAmplitude(i, 0);
            setOffset(i, 0);
            setDutyCycle(i, 0);
            setRun(args.channel, 0);
        }
    }

    if (strcmp(argv[1], "status") == 0)
    {
        getStatus();
    }
    printd();

    return EXIT_SUCCESS;
}