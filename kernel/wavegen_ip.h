// wavegen.h

#ifndef WAVEGEN_IP_H
#define WAVEGEN_IP_H

#include <stdint.h>
#include <stdbool.h>


bool waveGenOpen();

#endif 
void setChannelMode(volatile uint32_t channel,volatile uint32_t mode);
void setFrequency(volatile uint32_t channel,volatile uint32_t frequency);
void setDutyCycle(volatile uint32_t channel,volatile uint32_t dutyCycle);
void setAmplitude(volatile uint32_t channel,volatile uint32_t amplitude);
void setOffset(volatile uint32_t channel,volatile int32_t offset_fp);
void setCycles(volatile uint32_t channel,volatile uint32_t cycles);
void setRun(volatile uint32_t channel,volatile uint32_t run);
void getStatus();