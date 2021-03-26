#pragma once

int riceEncode(int* n, int size, int* output);
void riceDecode(unsigned char* input, int size, int* n);
void printRiceHist(void);
