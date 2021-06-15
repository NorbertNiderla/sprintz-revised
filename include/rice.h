#pragma once

int riceEncode(int* n, int size, int* output);
void riceDecode(int* input, int size, int* n, unsigned long long r);
int riceEncodeStream(int* n, int size, unsigned char* output, size_t output_buffer_size);
void riceDecodeStream(unsigned char* input, int size, int* n);
void printRiceHist(void);
