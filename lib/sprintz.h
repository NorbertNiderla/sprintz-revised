#pragma once

void sprintzDecode(unsigned char* input, size_t input_buffer_size, int* output, int size, int bytes);
int sprintzEncode(const int* input, int size, unsigned char* output, size_t output_buffer_size, bool new_huff_table);
void zigzag(int* data, int size);
