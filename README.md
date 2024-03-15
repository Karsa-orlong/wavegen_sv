# WaveGen System

## An attempt to create a Signal Generator using the Xilinx Blackboard FPGA device

## Supported functions
* DC
* Sine
* Sawtooth
* Triangle
* Square

## Supported configurations
### DC
1. Offset
2. Amplitude

### Sine
1. Offset
2. Amplitude
3. Frequency
4. Cycles
5. Phase

### Sawtooth
1. Offset
2. Amplitude
3. Frequency
4. Cycles

### Triangle
1. Offset
2. Amplitude
3. Frequency
4. Cycles
5. Phase

### Square
1. Offset
2. Amplitude
3. Frequency
4. Cycles
5. Duty Cycle


## Special features
1. Support for Hilbert's transform for Sine and Triangle signals
2. Differential waves on both channels
3. Phase shift for Sine and Triangle signals


## Execution
## Kernel module
* Kernel file present in:
    ~/C/kernel/wavegen_driver.c

1. cd ~/C/kernel/
2. make
3. sudo insmod wavegen_driver.ko


## Channel Update
cd /sys/kernel/wavegen/[channel]
 where [channel] is one of
 1. 0
 2. 1

## Mode Update
 echo [mode] > mode[channel]

 where [mode] can be one of
 1. dc
 2. sine
 3. saw
 4. sq
 5. tri

## Offset Update
 echo [offsetValue] > offset[channel]

 where [offsetValue] is a value between -2500 (for -2.5V) and 2500 (for 2.5V)


## Amplitude Update
 echo [amplitudeValue] > offset[channel]

 where [amplitudeValue] is a value between -2500 (for -2.5V) and 2500 (for 2.5V)


## Frequency Update
 echo [frequencyValue] > frequency[channel]

 where [frequencyValue] is a value between 0  and 8000 (will begin to show distortions)


## Duty Cycles Update
 echo [dutyCycleValue] > duty[channel]

 where [dutyCycleValue] is a value between 0 and 100


## Cycles Update
 echo [cycleValue] > duty[channel]

 where [cycleValue] == 0 = > Infinite cycles
       [cycleValue] == n = > n cycles


## Differential Update
 echo [mode] > comp[channel]

 where [mode] is a value "on" or "off"


## Phase Update
 echo [phaseValue] > duty[channel]

 where [phaseValue] is a value between 0 and 360


## Hilbert Update
 echo [mode] > duty[channel]

 where [mode] is a value "on" or "off"



## Run Update
 echo [channelMode] > run[channel]

 where [channelMode] is "a" = channel A
                        "b" = channel B
                        "c" = channel A+B
 Note: when independent, channels A and B can only configured when in the respective channel
            "c" is acceptable in both run0 and run1
