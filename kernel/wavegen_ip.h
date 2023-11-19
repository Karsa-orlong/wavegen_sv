// wavegen.h

#ifndef WAVEGEN_IP_H
#define WAVEGEN_IP_H

#include <stdint.h>
#include <stdbool.h>


bool waveGenOpen();

#endif 
void setChannelMode(uint32_t channel, uint32_t mode);
void setFrequency(uint32_t channel, uint32_t frequency);
void setDutyCycle(uint32_t channel, uint32_t dutyCycle);
void setAmplitude(uint32_t channel, uint32_t amplitude);
void setOffset(uint32_t channel, int32_t offset);
void setCycles(uint32_t channel, uint32_t cycles);
void setRun(uint32_t channel, uint32_t run);