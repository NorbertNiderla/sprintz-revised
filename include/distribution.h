#pragma once
#include <stdint.h>

#define DENS_RANGE (256)
#define DENS_ENCODE_BITS (8)

void setFreqs(unsigned char* input, int size, int max_value, int* freqs);
void setOcc(int* input, int size, int L, uint8_t* occ);
uint8_t setFrequencies(uint8_t* input, int size, uint16_t* freqs);
void setFrequencies_Dens(int size, uint16_t* freqs, uint8_t dens_int);
void resetDensCounter(void);

void setOccurrences(int* input, int size, int L, uint16_t* occ);
void setOccurrencesNormal(int* input, int size, int L, uint16_t* occ);
void setOccurrences_char(unsigned char* input, int size, int L, uint16_t* occ);
void setOccurrencesNormal_char(unsigned char* input, int size, int L, uint16_t* occ);
