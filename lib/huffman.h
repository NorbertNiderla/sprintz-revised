#pragma once
#include <stdbool.h>

void huffmanDecode(unsigned char* input,size_t input_buffer_size, unsigned char* output, int size);
int huffmanEncode(unsigned char* input, int size, unsigned char* output, size_t output_buffer_size);
void freeHuffmanObjects(void);
void buildTree(void);
void buildLookUpTable(void);
int huffmanEncodeWithTableDecision(unsigned char* input, int size, unsigned char* output, size_t output_buffer_size, bool new_table);
