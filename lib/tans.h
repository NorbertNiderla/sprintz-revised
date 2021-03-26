#include <stdbool.h>
#pragma once

void buildTable(void);

int tansEncode(int* n, int size, unsigned char* output, size_t output_buffer_size, bool new_table, int method);
void tansDecode(unsigned char* input, size_t output_buffer_size, int bits, int size, int* n);
