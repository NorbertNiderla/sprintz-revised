#pragma once
#include "fire.h"

void sprintzDecode(unsigned char* input, size_t input_buffer_size, int* output, int size, fire_coder_t* fire_state);
int sprintzEncode(const int* input, int size, unsigned char* output, size_t output_buffer_size, bool new_huff_table, fire_coder_t* fire_state);
int sprintzEncode_tans(const int* input, int size, unsigned char* output, size_t output_buffer_size, bool new_huff_table, fire_coder_t* fire_state);
void zigzag(int* data, int size);
