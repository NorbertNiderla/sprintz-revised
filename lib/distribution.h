#pragma once

void setFrequencies(unsigned char* input, int size, int* freqs);
void resetDensCounter(void);
int setOccurrences(int* input, int size, int L, uint16_t* occ);
int setOccurrencesNormal(int* input, int size, int L, uint16_t* occ);
