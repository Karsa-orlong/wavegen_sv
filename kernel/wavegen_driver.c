/**
 *      @file wavegen_driver.c
 *      @author Prithvi Bhat
 *      @brief Target Platform: Xilinx XUP Blackboard
 *               Hardware configuration:
 *               AXI4-Lite interface
 *               Mapped to offset of 0x20000
 *      @version 0.1
 *      @date 2023-11-24
 **/

#include <linux/kernel.h>   // kstrtouint
#include <linux/module.h>   // MODULE_ macros
#include <linux/init.h>     // __init
#include <linux/kobject.h>  // kobject, kobject_atribute,
                            // kobject_create_and_add, kobject_put
#include <asm/io.h>         // iowrite, ioread, ioremap_nocache (platform specific)
#include "../address_map.h" // overall memory map
#include "wavegen_regs.h"


// Kernel module information
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Prithvi Bhat");
MODULE_DESCRIPTION("Wavegen IP Driver");

#define MODE_DC     0
#define MODE_SIN    1
#define MODE_SAW    2
#define MODE_TRI    3
#define MODE_SQR    4
#define MODE_ARB    5

#define CHANNEL_A   0
#define CHANNEL_B   1
#define CHANNEL_AB  2
#define SCALE_CONSTANT (1 << 14)

static unsigned int *base = NULL;

char mode[10];

// Subroutines
/**
 *      @brief Function to set the MODE register
 *      @param channel in which to set
 *      @param mode to be set for the channel
 **/
void updateMode(int channel, int mode)
{
    unsigned int value = ioread32(base + OFS_MODE);

    if (channel == CHANNEL_A)
    {
        value = value & 0x38;     // Clear out the existing values

        if (mode == MODE_DC)  iowrite32(value | MODE_DC , (base + OFS_MODE));  // Mode for DC Wave
        if (mode == MODE_SIN) iowrite32(value | MODE_SIN, (base + OFS_MODE));  // Mode for DC Wave
        if (mode == MODE_SAW) iowrite32(value | MODE_SAW, (base + OFS_MODE));  // Mode for DC Wave
        if (mode == MODE_TRI) iowrite32(value | MODE_TRI, (base + OFS_MODE));  // Mode for DC Wave
        if (mode == MODE_SQR) iowrite32(value | MODE_SQR, (base + OFS_MODE));  // Mode for DC Wave

    }
    else if (channel == CHANNEL_B)
    {
        value = value & 0x07;     // Clear out the existing values

        if (mode == MODE_DC)  iowrite32(value | (MODE_DC  << 3), (base + OFS_MODE));  // Mode for DC Wave
        if (mode == MODE_SIN) iowrite32(value | (MODE_SIN << 3), (base + OFS_MODE));  // Mode for DC Wave
        if (mode == MODE_SAW) iowrite32(value | (MODE_SAW << 3), (base + OFS_MODE));  // Mode for DC Wave
        if (mode == MODE_TRI) iowrite32(value | (MODE_TRI << 3), (base + OFS_MODE));  // Mode for DC Wave
        if (mode == MODE_SQR) iowrite32(value | (MODE_SQR << 3), (base + OFS_MODE));  // Mode for DC Wave
    }

}

/**
 *      @brief Get the Mode register
 *      @return uint32_t register value
 **/
unsigned int getMode(void)
{
    return (ioread32(base + OFS_MODE));                                                 // Read current value
}

/**
*      @brief Function to set the RUN register
*      @param channel in which to set
**/
void updateRun(int channel, int run)
{
    unsigned int value = ioread32(base + OFS_RUN);                                      // Read current value

    value = value;

    if (run == 1)
    {
        if      (channel == CHANNEL_A)  iowrite32((value | 0x01), (base + OFS_RUN));    // Channel A
        else if (channel == CHANNEL_B)  iowrite32((value | 0x02), (base + OFS_RUN));    // Channel B
        else if (channel == CHANNEL_AB) iowrite32((value | 0x03), (base + OFS_RUN));    // Channel A+B
    }

    else if (run == 0)
    {
        if      (channel == CHANNEL_A)  iowrite32((value & 0x02), (base + OFS_RUN));    // Channel A
        else if (channel == CHANNEL_B)  iowrite32((value & 0x01), (base + OFS_RUN));    // Channel B
        else if (channel == CHANNEL_AB) iowrite32((value & 0x0C), (base + OFS_RUN));    // Channel A+B
    }

    return;
}

/**
*      @brief Get the Run register
*      @return uint32_t register value
**/
unsigned int getRun(void)
{
    uint32_t value = 0;
    value = ioread32(base + OFS_RUN) & 0x03; // Read current value

    value = value & 3;

    return value;
}

/**
 *      @brief Function to update the complement mode in in the run register
 *      @param channel channel to complement with other
 *      @param mode of complement
 **/
void updateComplement (uint8_t channel, uint8_t mode)
{
    uint32_t value = 0;
    value = ioread32(base + OFS_RUN);                                                   // Read current value

    if (channel == CHANNEL_A)
    {
        if (mode)   value = (value | 4);
        else        value = (value & ~4);

        iowrite32(value, (base + OFS_RUN));
    }
    else if (channel == CHANNEL_B)
    {
        if (mode)   value = (value | 8);
        else        value = (value & ~8);

        iowrite32(value, (base + OFS_RUN));
    }
}

/**
*      @brief Function update the frequency register
*      @param channel to set
*      @param frequency value to set
**/
void updateFrequency(int channel, unsigned int frequency)
{
    uint32_t value = 0;

    if      (channel == CHANNEL_A)  iowrite32(frequency, (base + OFS_FREQA));           // Channel A
    else if (channel == CHANNEL_B)  iowrite32(frequency, (base + OFS_FREQB));           // Channel B

    value = (ioread32(base + OFS_FREQA));               // Read current value
}

/**
 *      @brief Get the Frequency object
 *
 *      @param channel
 *      @return uint32_t
 **/
unsigned int getFrequency(int channel)
{
    uint32_t value;
    if      (channel == CHANNEL_A)  value = (ioread32(base + OFS_FREQA));               // Read current value
    else if (channel == CHANNEL_B)  value = (ioread32(base + OFS_FREQB));               // Read current value

    return value;
}

/**
*      @brief Function to update the offset register
*      @param channel to set
**/
void updateOffset(int channel, signed int offset)
{
    unsigned int value = ioread32(base + OFS_OFFSET);

    if (channel == CHANNEL_A)
    {
        value = value & 0xFFFF0000;                                                     // Clear the lower 16
        value = value | offset;
        iowrite32(value, (base + OFS_OFFSET));                                          // Channel A
    }
    else if (channel == CHANNEL_B)
    {
        value = value & 0x0000FFFF;                                                     // Clear the upper 16
        value = value | (offset << 16);
        iowrite32(value, (base + OFS_OFFSET));                                          // Channel B
    }
}

/**
 *      @brief Get the Offset object
 *      @return uint32_t
 **/
int getOffset(void)
{
    return (ioread32(base + OFS_OFFSET));                                               // Read current value
}

/**
*      @brief Function to set the amplitude register
*      @param channel to set
*      @param amplitude to set
**/
void updateAmplitude(int channel, signed int amplitude)
{
    unsigned int value = ioread32(base + OFS_AMPLITUDE);

    if (channel == CHANNEL_A)
    {
        value = value & 0xFFFF0000;                                                     // Clear the lower 16
        value = value | amplitude;
        iowrite32(value, (base + OFS_AMPLITUDE));                                       // Channel A
    }
    else if (channel == CHANNEL_B)
    {
        value = value & 0x0000FFFF;                                                     // Clear the upper 16
        value = value | (amplitude << 16);
        iowrite32(value, (base + OFS_AMPLITUDE));                                       // Channel B
    }
}

/**
 *      @brief Get the Amplitude object
 *      @return uint32_t
 **/
int32_t getAmplitude(void)
{
    return (ioread32(base + OFS_AMPLITUDE));                                            // Read current value
}

/**
*      @brief Function to set the duty cycle register
*      @param channel to be set
*      @param duty to be set
**/
void updateDutyCycles(int channel, unsigned int duty)
{
    unsigned int value = ioread32(base + OFS_DTYCYC);

    if (channel == CHANNEL_A)
    {
        value = value & 0xFFFF0000;
        iowrite32(value | duty, (base + OFS_DTYCYC));                                   // Channel A
    }
    else if (channel == CHANNEL_B)
    {
        value = value & 0x0000FFFF;
        iowrite32((value | (duty << 16)), (base + OFS_DTYCYC));                         // Channel B
    }
}

/**
 *      @brief Get the Duty cycles object
 *      @return uint32_t
 **/
uint32_t getDutyCycles(void)
{
    return (ioread32(base + OFS_DTYCYC));                                               // Read current value
}

/**
*      @brief Function to set the Cycles register
*      @param channel to set
*      @param cycles value to set
**/
void updateCycles(int channel, unsigned int cycles)
{
    unsigned int value = ioread32(base + OFS_CYCLES);

    if (channel == CHANNEL_A)
    {
        value = value & 0xFFFF0000;
        iowrite32(value | cycles, (base + OFS_CYCLES));                                 // Channel A
    }
    else if (channel == CHANNEL_B)
    {
        value = value & 0x0000FFFF;
        iowrite32((value | (cycles << 16)), (base + OFS_CYCLES));                       // Channel B
    }
}

/**
 *      @brief Get the Cycles object
 *      @return uint32_t
 **/
uint32_t getCycles(void)
{
    return (ioread32(base + OFS_CYCLES));                                               // Read current value
}

int32_t signAndScale(int32_t value, int32_t divisor)
{
    int32_t scaledValue, signedScaled;

    scaledValue = (value * SCALE_CONSTANT);

    signedScaled = scaledValue / divisor;

    return signedScaled;
}

/**
 *      @brief Function to set the phase offset value into registers
 *                (Registers repurposed from other functions)
 *      @param channel to update
 *      @param phase value to set
 **/
void updatePhase (int channel, uint16_t phase)
{
    unsigned int value;

    if (channel == CHANNEL_A)
    {
        value = ioread32(base + OFS_MODE);
        value = value & 0x0000FFFF;
        iowrite32((value | (phase << 16)), (base + OFS_MODE));                          // Channel A
    }
    else if (channel == CHANNEL_B)
    {
        value = ioread32(base + OFS_RUN);
        value = value & 0x0000FFFF;
        iowrite32((value | (phase << 16)), (base + OFS_RUN));                           // Channel B
    }
}

uint16_t getPhase(int8_t channel)
{
    if      (channel == CHANNEL_A)   return ((ioread32(base + OFS_MODE) & 0xFFFF0000) >> 16);
    else if (channel == CHANNEL_B)   return ((ioread32(base + OFS_RUN) & 0xFFFF0000) >> 16);

    return 0;
}

/**
 *      @brief Construct a new update Hilbert object
 *      @param channel to update
 *      @param hilbertMode value to set
 **/
void updateHilbert(int8_t channel, int8_t hilbertMode)
{
    unsigned int value;
    value = ioread32(base + OFS_MODE);

    if (channel == CHANNEL_A)
    {
        if (hilbertMode)    value = value | 64;
        else                value = value & ~64;
    }

    else if (channel == CHANNEL_B)
    {
        if (hilbertMode)    value = value | 128;
        else                value = value & ~128;
    }

    iowrite32(value, (base + OFS_MODE));                                    // Channel A
}

uint8_t getHilbert(void)
{
    return ioread32(base + OFS_MODE) & 0x192;
}

//-----------------------------------------------------------------------------
// Kernel Objects
//-----------------------------------------------------------------------------

////////////////////////////////////////// Hilbert 0 //////////////////////////////////////////
static int hilbert0 = 0;
module_param(hilbert0, int, S_IRUGO);
MODULE_PARM_DESC(hilbert0, " Hilbert transform of waves");

/**
 *      @brief Kernel object function to store the hilbert value for channel A
 *      @param kobj
 *      @param attr
 *      @param buffer
 *      @param count
 *      @return ssize_t
 **/
static ssize_t hilbert0Store(struct kobject *kobj, struct kobj_attribute *attr, const char *buffer, size_t count)
{
    if (strncmp(buffer, "on", 2) == 0)                           // Channel A
    {
        updateHilbert(CHANNEL_A, 1);
        hilbert0 = 1;
        printk(KERN_INFO "Hilbert on on Channel A\n");
    }

    else if (strncmp(buffer, "off", 3) == 0)                    // Channel A
    {
        updateHilbert(CHANNEL_A, 0);
        hilbert0 = 0;
        printk(KERN_INFO "Hilbert off on Channel A\n");
    }

    return count;
}

/**
 *      @brief Kernel object function to get the hilbert value
 *
 *      @param kobj
 *      @param attr
 *      @param buffer
 *      @return ssize_t
 **/
static ssize_t hilbert0Show(struct kobject *kobj, struct kobj_attribute *attr, char *buffer)
{
    if      (hilbert0 == 0)   strcpy(buffer, "Hilbert OFF\n");
    else if (hilbert0 == 1)   strcpy(buffer, "Hilbert ON\n");

    return strlen(buffer);
}

static struct kobj_attribute hilbert0Attr = __ATTR(hilbert0, 0664, hilbert0Show, hilbert0Store);




////////////////////////////////////////// Hilbert 1 //////////////////////////////////////////
static int hilbert1 = 0;
module_param(hilbert1, int, S_IRUGO);
MODULE_PARM_DESC(hilbert1, " Hilbert transform of waves");

/**
 *      @brief Kernel object function to store the hilbert value for channel B
 *      @param kobj
 *      @param attr
 *      @param buffer
 *      @param count
 *      @return ssize_t
 **/
static ssize_t hilbert1Store(struct kobject *kobj, struct kobj_attribute *attr, const char *buffer, size_t count)
{
    if (strncmp(buffer, "on", 2) == 0)                           // Channel A
    {
        updateHilbert(CHANNEL_B, 1);
        hilbert1 = 1;
        printk(KERN_INFO "Hilbert on on Channel B\n");
    }

    else if (strncmp(buffer, "off", 3) == 0)                    // Channel A
    {
        updateHilbert(CHANNEL_B, 0);
        hilbert1 = 0;
        printk(KERN_INFO "Hilbert off on Channel B\n");
    }

    return count;
}

/**
 *      @brief Kernel object function to get the hilbert value
 *
 *      @param kobj
 *      @param attr
 *      @param buffer
 *      @return ssize_t
 **/
static ssize_t hilbert1Show(struct kobject *kobj, struct kobj_attribute *attr, char *buffer)
{
    if      (hilbert1 == 0)   strcpy(buffer, "Hilbert OFF\n");
    else if (hilbert1 == 1)   strcpy(buffer, "Hilbert ON\n");

    return strlen(buffer);
}

static struct kobj_attribute hilbert1Attr = __ATTR(hilbert1, 0664, hilbert1Show, hilbert1Store);


////////////////////////////////////////// Run 0 //////////////////////////////////////////
static int run0 = 0;
module_param(run0, int, S_IRUGO);
MODULE_PARM_DESC(run0, " Run waves");

/**
 *      @brief Kernel object function to store the run value for channel A
 *      @param kobj
 *      @param attr
 *      @param buffer
 *      @param count
 *      @return ssize_t
 **/
static ssize_t run0Store(struct kobject *kobj, struct kobj_attribute *attr, const char *buffer, size_t count)
{
    if (strncmp(buffer, "a", 1) == 0)                           // Channel A
    {
        updateRun(CHANNEL_A, 1);
        run0 = 0;
        printk(KERN_INFO "Running A\n");
    }

    else if (strncmp(buffer, "c", 1) == 0)                      // Channel A+B
    {
        updateRun(CHANNEL_AB, 1);
        run0 = 2;
        printk(KERN_INFO "Running A+B\n");
    }

    else if (strncmp(buffer, "stop", strlen("stop")) == 0) // Clear Channel A+B
    {
        updateRun(CHANNEL_AB, 0);
        run0 = 1;
        printk(KERN_INFO "Stopped A+B\n");
    }
    return count;
}

/**
 *      @brief Kernel object function to get the run value
 *
 *      @param kobj
 *      @param attr
 *      @param buffer
 *      @return ssize_t
 **/
static ssize_t run0Show(struct kobject *kobj, struct kobj_attribute *attr, char *buffer)
{
    if      (run0 == 0)   strcpy(buffer, "Channel A\n");
    else if (run0 == 2)   strcpy(buffer, "Channel A+B\n");

    return strlen(buffer);
}

static struct kobj_attribute run0Attr = __ATTR(run0, 0664, run0Show, run0Store);

////////////////////////////////////////// Run 1 //////////////////////////////////////////
static int run1 = 0;
module_param(run1, int, S_IRUGO);
MODULE_PARM_DESC(run1, " Run waves");

/**
 *      @brief Kernel object function to store the run value for channel B
 *      @param kobj
 *      @param attr
 *      @param buffer
 *      @param count
 *      @return ssize_t
 **/
static ssize_t run1Store(struct kobject *kobj, struct kobj_attribute *attr, const char *buffer, size_t count)
{
    if (strncmp(buffer, "b", 1) == 0)                           // Channel B
    {
        updateRun(CHANNEL_B, 1);
        run1 = 0;
        printk(KERN_INFO "Running B\n");
    }

    else if (strncmp(buffer, "c", 1) == 0)                      // Channel A+B
    {
        updateRun(CHANNEL_AB, 1);
        run1 = 2;
        printk(KERN_INFO "Running A+B\n");
    }

    else if (strncmp(buffer, "stop", strlen("stop")) == 0) // Clear Channel A+B
    {
        updateRun(CHANNEL_AB, 0);
        run1 = 1;
        printk(KERN_INFO "Stopped A+B\n");
    }
    return count;
}

/**
 *      @brief Kernel object function to get the run value
 *
 *      @param kobj
 *      @param attr
 *      @param buffer
 *      @return ssize_t
 **/
static ssize_t run1Show(struct kobject *kobj, struct kobj_attribute *attr, char *buffer)
{
    if (run1 == 0)      strcpy(buffer, "Channel B\n");
    else if (run1 == 2) strcpy(buffer, "Channel A+B\n");

    return strlen(buffer);
}

static struct kobj_attribute run1Attr = __ATTR(run1, 0664, run1Show, run1Store);


////////////////////////////////////////// Complementary 0 //////////////////////////////////////////
static int comp0 = 0;
module_param(comp0, int, S_IRUGO);
MODULE_PARM_DESC(comp0, " Complement Waves");

/**
 *      @brief Kernel object function to store the complement logic value for channel A
 *      @param kobj
 *      @param attr
 *      @param buffer
 *      @param count
 *      @return ssize_t
 **/
static ssize_t comp0Store(struct kobject *kobj, struct kobj_attribute *attr, const char *buffer, size_t count)
{
    if (strncmp(buffer, "on", 2) == 0)                              // On
    {
        updateComplement(CHANNEL_A, 1);
        comp0 = 0;
        printk(KERN_INFO "Complementing A with B\n");
    }

    else if (strncmp(buffer, "off", 3) == 0)                        // Off
    {
        updateComplement(CHANNEL_A, 0);
        comp0 = 0;
        printk(KERN_INFO "Independent waves on A and B\n");
    }
    return count;
}

/**
 *      @brief Kernel object function to get the complement value
 *
 *      @param kobj
 *      @param attr
 *      @param buffer
 *      @return ssize_t
 **/
static ssize_t comp0Show(struct kobject *kobj, struct kobj_attribute *attr, char *buffer)
{
    if (comp0 == 1)      strcpy(buffer, "Complementing A with B\n");
    else if (comp0 == 0) strcpy(buffer, "Independent waves\n");

    return strlen(buffer);
}

static struct kobj_attribute comp0Attr = __ATTR(comp0, 0664, comp0Show, comp0Store);



////////////////////////////////////////// Complementary 1 //////////////////////////////////////////
static int comp1 = 0;
module_param(comp1, int, S_IRUGO);
MODULE_PARM_DESC(comp1, " Complement Waves");

/**
 *      @brief Kernel object function to store the complement logic value for channel B
 *      @param kobj
 *      @param attr
 *      @param buffer
 *      @param count
 *      @return ssize_t
 **/
static ssize_t comp1Store(struct kobject *kobj, struct kobj_attribute *attr, const char *buffer, size_t count)
{
    if (strncmp(buffer, "on", 2) == 0)                              // On
    {
        updateComplement(CHANNEL_B, 1);
        comp0 = 0;
        printk(KERN_INFO "Complementing B with A\n");
    }

    else if (strncmp(buffer, "off", 3) == 0)                        // Off
    {
        updateComplement(CHANNEL_B, 0);
        comp0 = 0;
        printk(KERN_INFO "Independent waves on A and B\n");
    }
    return count;
}

/**
 *      @brief Kernel object function to get the complement value
 *
 *      @param kobj
 *      @param attr
 *      @param buffer
 *      @return ssize_t
 **/
static ssize_t comp1Show(struct kobject *kobj, struct kobj_attribute *attr, char *buffer)
{
    if (comp1 == 1)      strcpy(buffer, "Complementing B with A\n");
    else if (comp1 == 0) strcpy(buffer, "Independent waves\n");

    return strlen(buffer);
}

static struct kobj_attribute comp1Attr = __ATTR(comp1, 0664, comp1Show, comp1Store);



////////////////////////////////////////// Mode 0 //////////////////////////////////////////
static int mode0 = 0;
// module_param(mode0, bool, S_IRUGO);
MODULE_PARM_DESC(mode0, " Mode of channel A");

/**
*      @brief Kernel Object function to set the mode register for channel A
*      @param kobj
*      @param attr
*      @param buffer
*      @param count
*      @return ssize_t
**/
static ssize_t mode0Store(struct kobject *kobj, struct kobj_attribute *attr, const char *buffer, size_t count)
{
    if (strncmp(buffer, "dc", strlen("dc")) == 0)
    {
        mode0 = MODE_DC;
        updateMode(CHANNEL_A, MODE_DC);
    }
    if (strncmp(buffer, "sine", strlen("sine")) == 0)
    {
        mode0 = MODE_SIN;
        updateMode(CHANNEL_A, MODE_SIN);
    }
    if (strncmp(buffer, "saw", strlen("saw")) == 0)
    {
        mode0 = MODE_SAW;
        updateMode(CHANNEL_A, MODE_SAW);
    }
    if (strncmp(buffer, "tri", strlen("tri")) == 0)
    {
        mode0 = MODE_TRI;
        updateMode(CHANNEL_A, MODE_TRI);
    }
    if (strncmp(buffer, "sq", strlen("sq")) == 0)
    {
        mode0 = MODE_SQR;
        updateMode(CHANNEL_A, MODE_SQR);
    }

    return count;
}

/**
 *      @brief Kernel object function to get the mode value for channel A
 *      @param kobj
 *      @param attr
 *      @param buffer
 *      @return ssize_t
 **/
static ssize_t mode0Show(struct kobject *kobj, struct kobj_attribute *attr, char *buffer)
{
    mode0 = getMode();

    if      (mode0 & 0x07 & MODE_DC)    strcpy(buffer, "DC\n");
    else if (mode0 & 0x07 & MODE_SIN)   strcpy(buffer, "Sine\n");
    else if (mode0 & 0x07 & MODE_SAW)   strcpy(buffer, "Sawtooth\n");
    else if (mode0 & 0x07 & MODE_TRI)   strcpy(buffer, "Triangle\n");
    else if (mode0 & 0x07 & MODE_SQR)   strcpy(buffer, "Square\n");

    return strlen(buffer);
}

static struct kobj_attribute mode0Attr = __ATTR(mode0, 0664, mode0Show, mode0Store);



////////////////////////////////////////// Mode 1 //////////////////////////////////////////
static int mode1 = 0;
MODULE_PARM_DESC(mode1, " Mode of channel B");

/**
*      @brief Kernel Object function to set the mode register for channel B
*      @param kobj
*      @param attr
*      @param buffer
*      @param count
*      @return ssize_t
**/
static ssize_t mode1Store(struct kobject *kobj, struct kobj_attribute *attr, const char *buffer, size_t count)
{
    if (strncmp(buffer, "dc", strlen("dc")) == 0)
    {
        mode1 = MODE_DC;
        updateMode(CHANNEL_B, MODE_DC);
    }
    if (strncmp(buffer, "sine", strlen("sine")) == 0)
    {
        mode1 = MODE_SIN;
        updateMode(CHANNEL_B, MODE_SIN);
    }
    if (strncmp(buffer, "saw", strlen("saw")) == 0)
    {
        mode1 = MODE_SAW;
        updateMode(CHANNEL_B, MODE_SAW);
    }
    if (strncmp(buffer, "tri", strlen("tri")) == 0)
    {
        mode1 = MODE_TRI;
        updateMode(CHANNEL_B, MODE_TRI);
    }
    if (strncmp(buffer, "sq", strlen("sq")) == 0)
    {
        mode1 = MODE_SQR;
        updateMode(CHANNEL_B, MODE_SQR);
    }

    return count;
}

/**
 *      @brief Kernel object function to get the mode value for channel A
 *      @param kobj
 *      @param attr
 *      @param buffer
 *      @return ssize_t
 **/
static ssize_t mode1Show(struct kobject *kobj, struct kobj_attribute *attr, char *buffer)
{
    mode1 = getMode();

    if      (mode1 & 0x56 & MODE_DC)    strcpy(buffer, "DC\n");
    else if (mode1 & 0x56 & MODE_SIN)   strcpy(buffer, "Sine\n");
    else if (mode1 & 0x56 & MODE_SAW)   strcpy(buffer, "Sawtooth\n");
    else if (mode1 & 0x56 & MODE_TRI)   strcpy(buffer, "Triangle\n");
    else if (mode1 & 0x56 & MODE_SQR)   strcpy(buffer, "Square\n");

    return strlen(buffer);
}

static struct kobj_attribute mode1Attr = __ATTR(mode1, 0664, mode1Show, mode1Store);



////////////////////////////////////////// Frequency 0 //////////////////////////////////////////
static int frequency0 = 0;
// module_param(frequency0, int, S_IRUGO);
MODULE_PARM_DESC(frequency0, " Frequency of Channel A");

/**
 *      @brief Kernel object function to update the frequency register for channel A
 *      @param kobj
 *      @param attr
 *      @param buffer
 *      @param count
 *      @return ssize_t
 **/
static ssize_t frequency0Store(struct kobject *kobj, struct kobj_attribute *attr, const char *buffer, size_t count)
{
    uint32_t result = kstrtouint(buffer, 0, &frequency0);

    if (!result)    updateFrequency(CHANNEL_A, frequency0);

    return count;
}

/**
 *      @brief Kernel object function to read the frequency register for Channel A
 *
 *      @param kobj
 *      @param attr
 *      @param buffer
 *      @return ssize_t
 **/
static ssize_t frequency0Show(struct kobject *kobj, struct kobj_attribute *attr, char *buffer)
{
    uint32_t result;
    result = getFrequency(CHANNEL_A);

    return sprintf(buffer, "%d\n", result);
}

static struct kobj_attribute frequency0Attr = __ATTR(frequency0, 0664, frequency0Show, frequency0Store);



////////////////////////////////////////// Frequency 1 //////////////////////////////////////////
static int frequency1 = 0;
// module_param(frequency1, int, S_IRUGO);
MODULE_PARM_DESC(frequency1, " Frequency of Channel B");

/**
 *      @brief Kernel object function to update the frequency register for channel B
 *      @param kobj
 *      @param attr
 *      @param buffer
 *      @param count
 *      @return ssize_t
 **/
static ssize_t frequency1Store(struct kobject *kobj, struct kobj_attribute *attr, const char *buffer, size_t count)
{
    uint32_t result = kstrtouint(buffer, 0, &frequency1);

    if (!result)    updateFrequency(CHANNEL_B, frequency1);

    return count;
}

/**
 *      @brief Kernel object function to read the frequency register for Channel B
 *      @param kobj
 *      @param attr
 *      @param buffer
 *      @return ssize_t
 **/
static ssize_t frequency1Show(struct kobject *kobj, struct kobj_attribute *attr, char *buffer)
{
    uint32_t result;
    result = getFrequency(CHANNEL_B);

    return sprintf(buffer, "%d\n", result);
}

static struct kobj_attribute frequency1Attr = __ATTR(frequency1, 0664, frequency1Show, frequency1Store);




////////////////////////////////////////// Offset 0 //////////////////////////////////////////
static int offset0 = 0;
// module_param(offset0, float, S_IRUGO);
MODULE_PARM_DESC(offset0, " Offset of Channel A");

/**
 *      @brief kernel object function to set the offset register
 *
 *      @param kobj
 *      @param attr
 *      @param buffer
 *      @param count
 *      @return ssize_t
 **/
static ssize_t offset0Store(struct kobject *kobj, struct kobj_attribute *attr, const char *buffer, size_t count)
{
    int32_t signedScaled;
    sscanf(buffer, "%d", &offset0);

    signedScaled = (signAndScale(offset0, 2500) & 0x0000FFFF);

    printk(KERN_INFO "Set: %d\n", signedScaled);

    updateOffset(CHANNEL_A, signedScaled);

    return count;
}

/**
 *      @brief Kernel object function to read offset register for Channel A
 *
 *      @param kobj
 *      @param attr
 *      @param buffer
 *      @return ssize_t
 **/
static ssize_t offset0Show(struct kobject *kobj, struct kobj_attribute *attr, char *buffer)
{
    int channelA_offset;

    channelA_offset = getOffset();

    sprintf(buffer, "%d\n", offset0);

    return strlen(buffer);
}

static struct kobj_attribute offset0Attr = __ATTR(offset0, 0664, offset0Show, offset0Store);


////////////////////////////////////////// Offset 1 //////////////////////////////////////////
static int offset1 = 0;
// module_param(offset1, float, S_IRUGO);
MODULE_PARM_DESC(offset1, " Offset of Channel B");

/**
 *      @brief kernel object function to set the offset register
 *
 *      @param kobj
 *      @param attr
 *      @param buffer
 *      @param count
 *      @return ssize_t
 **/
static ssize_t offset1Store(struct kobject *kobj, struct kobj_attribute *attr, const char *buffer, size_t count)
{
    int32_t signedScaled;
    sscanf(buffer, "%d", &offset1);

    signedScaled = signAndScale(offset1, 2500);

    printk(KERN_INFO "Set: %d\n", signedScaled);

    updateOffset(CHANNEL_B, signedScaled);

    return count;
}

/**
 *      @brief Kernel object function to read offset register for Channel A
 *
 *      @param kobj
 *      @param attr
 *      @param buffer
 *      @return ssize_t
 **/
static ssize_t offset1Show(struct kobject *kobj, struct kobj_attribute *attr, char *buffer)
{
    int channelA_offset;

    channelA_offset = getOffset();

    sprintf(buffer, "%d\n", offset1);

    return strlen(buffer);
}

static struct kobj_attribute offset1Attr = __ATTR(offset1, 0664, offset1Show, offset1Store);



////////////////////////////////////////// Amplitude 0 //////////////////////////////////////////
static int amplitude0 = 0;
// module_param(amplitude0, float, S_IRUGO);
MODULE_PARM_DESC(amplitude0, " Amplitude of Channel A");

/**
 *      @brief Kernel object function to set the Amplitude register on channel A
 *
 *      @param kobj
 *      @param attr
 *      @param buffer
 *      @param count
 *      @return ssize_t
 **/
static ssize_t amplitude0Store(struct kobject *kobj, struct kobj_attribute *attr, const char *buffer, size_t count)
{
    int32_t signedScaled;
    sscanf(buffer, "%d", &amplitude0);

    signedScaled = (signAndScale(amplitude0, 2500) & 0x0000FFFF);

    printk(KERN_INFO "Set: %d\n", signedScaled);

    updateAmplitude(CHANNEL_A, signedScaled);
    return count;
}

/**
 *      @brief Kernel object function to read Amplitude register for Channel A
 *
 *      @param kobj
 *      @param attr
 *      @param buffer
 *      @return ssize_t
 **/
static ssize_t amplitude0Show(struct kobject *kobj, struct kobj_attribute *attr, char *buffer)
{
    sprintf(buffer, "%d\n", amplitude0);

    return strlen(buffer);
}

static struct kobj_attribute amplitude0Attr = __ATTR(amplitude0, 0664, amplitude0Show, amplitude0Store);


////////////////////////////////////////// Amplitude 1 //////////////////////////////////////////
static int amplitude1 = 0;
// module_param(amplitude1, float, S_IRUGO);
MODULE_PARM_DESC(amplitude1, " Amplitude of Channel B");

/**
 *      @brief Kernel object function to set the Amplitude register on channel B
 *
 *      @param kobj
 *      @param attr
 *      @param buffer
 *      @param count
 *      @return ssize_t
 **/
static ssize_t amplitude1Store(struct kobject *kobj, struct kobj_attribute *attr, const char *buffer, size_t count)
{
    int32_t signedScaled;
    sscanf(buffer, "%d", &amplitude1);

    signedScaled = signAndScale(amplitude1, 2500);

    printk(KERN_INFO "Set: %d\n", signedScaled);

    updateAmplitude(CHANNEL_B, signedScaled);
    return count;
}

/**
 *      @brief Kernel object function to read Amplitude register for Channel B
 *
 *      @param kobj
 *      @param attr
 *      @param buffer
 *      @return ssize_t
 **/
static ssize_t amplitude1Show(struct kobject *kobj, struct kobj_attribute *attr, char *buffer)
{
    sprintf(buffer, "%d\n", amplitude1);

    return strlen(buffer);
}

static struct kobj_attribute amplitude1Attr = __ATTR(amplitude1, 0664, amplitude1Show, amplitude1Store);



////////////////////////////////////////// Duty Cycles 0 //////////////////////////////////////////
static int duty0 = 0;
// module_param(duty0, float, S_IRUGO);
MODULE_PARM_DESC(duty0, " Duty Cycles of Channel A");

/**
 *      @brief Kernel object function to set the Duty cycles register on channel A
 *
 *      @param kobj
 *      @param attr
 *      @param buffer
 *      @param count
 *      @return ssize_t
 **/
static ssize_t duty0Store(struct kobject *kobj, struct kobj_attribute *attr, const char *buffer, size_t count)
{
    int32_t signedScaled;
    sscanf(buffer, "%d", &duty0);

    signedScaled = (uint32_t)signAndScale(duty0, 100);

    printk(KERN_INFO "Set: %d\n", signedScaled);

    updateDutyCycles(CHANNEL_A, signedScaled);
    return count;
}

/**
 *      @brief Kernel object function to read Duty Cycles register for Channel A
 *
 *      @param kobj
 *      @param attr
 *      @param buffer
 *      @return ssize_t
 **/
static ssize_t duty0Show(struct kobject *kobj, struct kobj_attribute *attr, char *buffer)
{
    sprintf(buffer, "%d", duty0);

    return strlen(buffer);
}

static struct kobj_attribute duty0Attr = __ATTR(duty0, 0664, duty0Show, duty0Store);


////////////////////////////////////////// Duty Cycles 1 //////////////////////////////////////////
static int duty1 = 0;
// module_param(duty1, float, S_IRUGO);
MODULE_PARM_DESC(duty1, " Duty Cycles of Channel B");

/**
 *      @brief Kernel object function to set the Duty cycles register on channel B
 *
 *      @param kobj
 *      @param attr
 *      @param buffer
 *      @param count
 *      @return ssize_t
 **/
static ssize_t duty1Store(struct kobject *kobj, struct kobj_attribute *attr, const char *buffer, size_t count)
{
    uint32_t signedScaled;
    sscanf(buffer, "%d", &duty1);

    signedScaled = (uint32_t)signAndScale(duty1, 100);

    printk(KERN_INFO "Set: %d\n", signedScaled);

    updateDutyCycles(CHANNEL_B, signedScaled);
    return count;
}

/**
 *      @brief Kernel object function to read Duty Cycles register for Channel B
 *
 *      @param kobj
 *      @param attr
 *      @param buffer
 *      @return ssize_t
 **/
static ssize_t duty1Show(struct kobject *kobj, struct kobj_attribute *attr, char *buffer)
{
    sprintf(buffer, "%d", duty1);

    return strlen(buffer);
}

static struct kobj_attribute duty1Attr = __ATTR(duty1, 0664, duty1Show, duty1Store);



////////////////////////////////////////// Cycles 0 //////////////////////////////////////////
static int cycles0 = 0;
// module_param(cycles0, int, S_IRUGO);
MODULE_PARM_DESC(cycles0, " Cycles of Channel A");

/**
 *      @brief Kernel object function to set the Cycles register on channel A
 *
 *      @param kobj
 *      @param attr
 *      @param buffer
 *      @param count
 *      @return ssize_t
 **/
static ssize_t cycles0Store(struct kobject *kobj, struct kobj_attribute *attr, const char *buffer, size_t count)
{
    sscanf(buffer, "%d", &cycles0);

    printk(KERN_INFO "Set: %d\n", cycles0);

    updateCycles(CHANNEL_A, cycles0);
    return count;
}

/**
 *      @brief Kernel object function to read Cycles register for Channel A
 *
 *      @param kobj
 *      @param attr
 *      @param buffer
 *      @return ssize_t
 **/
static ssize_t cycles0Show(struct kobject *kobj, struct kobj_attribute *attr, char *buffer)
{
    sprintf(buffer, "%d", cycles0);

    return strlen(buffer);
}

static struct kobj_attribute cycles0Attr = __ATTR(cycles0, 0664, cycles0Show, cycles0Store);


////////////////////////////////////////// Cycles 1 //////////////////////////////////////////
static int cycles1 = 0;
// module_param(cycles1, int, S_IRUGO);
MODULE_PARM_DESC(cycles1, " Cycles of Channel B");

/**
 *      @brief Kernel object function to set the Cycles register on channel B
 *
 *      @param kobj
 *      @param attr
 *      @param buffer
 *      @param count
 *      @return ssize_t
 **/
static ssize_t cycles1Store(struct kobject *kobj, struct kobj_attribute *attr, const char *buffer, size_t count)
{
    sscanf(buffer, "%d", &cycles1);

    printk(KERN_INFO "Set: %d\n", cycles1);

    updateCycles(CHANNEL_B, cycles1);
    return count;
}

/**
 *      @brief Kernel object function to read Cycles register for Channel B
 *
 *      @param kobj
 *      @param attr
 *      @param buffer
 *      @return ssize_t
 **/
static ssize_t cycles1Show(struct kobject *kobj, struct kobj_attribute *attr, char *buffer)
{
    sprintf(buffer, "%d", cycles1);

    return strlen(buffer);
}

static struct kobj_attribute cycles1Attr = __ATTR(cycles1, 0664, cycles1Show, cycles1Store);


////////////////////////////////////////// Phase 0 //////////////////////////////////////////
static int phase0 = 0;
MODULE_PARM_DESC(phase0, " Phase of Channel A");

/**
 *      @brief Function to set the phase value for Channel A
 *      @param kobj
 *      @param attr
 *      @param buffer
 *      @param count
 *      @return ssize_t
 **/
static ssize_t phase0Store(struct kobject *kobj, struct kobj_attribute *attr, const char *buffer, size_t count)
{
    uint32_t signedScaled;

    sscanf(buffer, "%d", &phase0);

    signedScaled = (phase0 << 12) / 360;

    printk(KERN_INFO "Set: %d\n", phase0);

    // signedScaled = (uint16_t)signAndScale(phase0, 360) & 0xFFFF;

    updatePhase(CHANNEL_A, signedScaled);

    return count;
}

/**
 *      @brief Function to read the phase value from register for Channel A
 *      @param kobj
 *      @param attr
 *      @param buffer
 *      @return ssize_t
 **/
static ssize_t phase0Show(struct kobject *kobj, struct kobj_attribute *attr, char *buffer)
{
    sprintf(buffer, "%d", phase0);

    return strlen(buffer);
}

static struct kobj_attribute phase0Attr = __ATTR(phase0, 0664, phase0Show, phase0Store);


////////////////////////////////////////// Phase 1 //////////////////////////////////////////
static int phase1 = 0;
MODULE_PARM_DESC(phase1, " Phase of Channel B");

/**
 *      @brief Function to set the phase value for Channel B
 *      @param kobj
 *      @param attr
 *      @param buffer
 *      @param count
 *      @return ssize_t
 **/
static ssize_t phase1Store(struct kobject *kobj, struct kobj_attribute *attr, const char *buffer, size_t count)
{
    uint32_t signedScaled;

    sscanf(buffer, "%d", &phase1);

    signedScaled = (phase1 << 12) / 360;

    printk(KERN_INFO "Set: %d\n", phase1);

    // signedScaled = (uint16_t)signAndScale(phase1, 360) & 0xFFFF;

    updatePhase(CHANNEL_B, signedScaled);
    return count;
}

/**
 *      @brief Function to read the phase value from register for Channel B
 *      @param kobj
 *      @param attr
 *      @param buffer
 *      @return ssize_t
 **/
static ssize_t phase1Show(struct kobject *kobj, struct kobj_attribute *attr, char *buffer)
{
    sprintf(buffer, "%d", phase1);

    return strlen(buffer);
}

static struct kobj_attribute phase1Attr = __ATTR(phase1, 0664, phase1Show, phase1Store);






// Attributes
static struct attribute *attrs0[] =
    {
        &mode0Attr.attr,
        &run0Attr.attr,
        &amplitude0Attr.attr,
        &offset0Attr.attr,
        &duty0Attr.attr,
        &cycles0Attr.attr,
        &frequency0Attr.attr,
        &phase0Attr.attr,
        &comp0Attr.attr,
        &hilbert0Attr.attr,
        NULL
    };
static struct attribute *attrs1[] =
    {
        &mode1Attr.attr,
        &run1Attr.attr,
        &amplitude1Attr.attr,
        &offset1Attr.attr,
        &duty1Attr.attr,
        &cycles1Attr.attr,
        &frequency1Attr.attr,
        &phase1Attr.attr,
        &comp1Attr.attr,
        &hilbert1Attr.attr,
        NULL
    };

static struct attribute_group group0 =
    {
        .name = "0",
        .attrs = attrs0
    };

static struct attribute_group group1 =
    {
        .name = "1",
        .attrs = attrs1
    };

static struct kobject *kobj;

//-----------------------------------------------------------------------------
// Initialization and Exit
//-----------------------------------------------------------------------------

static int __init initialize_module(void)
{
    int result;

    printk(KERN_INFO "Wavegen driver: starting\n");

    // Create wavegen directory under /sys/kernel
    kobj = kobject_create_and_add("wavegen", kernel_kobj);
    if (!kobj)
    {
        printk(KERN_ALERT "Wavegen driver: failed to create and add kobj\n");
        return -ENOENT;
    }

    // Create wavegen0 and wavegen1 groups
    result = sysfs_create_group(kobj, &group0);
    if (result != 0)    return result;

    result = sysfs_create_group(kobj, &group1);
    if (result != 0)    return result;

    // Physical to virtual memory map to access gpio registers
    base = (unsigned int *)ioremap(AXI4_LITE_BASE + WAVEGEN_IP_OFFSET, SPAN_IN_BYTES);

    if (base == NULL)   return -ENODEV;

    printk(KERN_INFO "Wavegen driver: initialized\n");

    return 0;
}

static void __exit exit_module(void)
{
    kobject_put(kobj);
    printk(KERN_INFO "Wavegen driver: exit\n");
}

module_init(initialize_module);
module_exit(exit_module);
