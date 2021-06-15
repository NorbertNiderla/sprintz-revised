/*
 * huffman_coding.h
 *
 *  Created on: May 5, 2021
 *      Author: Norbert Niderla
 */

#ifndef HUFFMAN_CODING_H_
#define HUFFMAN_CODING_H_

void huffman_decode(unsigned char* input, size_t input_buffer_size, uint8_t* n);
int huffman_encode(unsigned char* input, int size, unsigned char* output, size_t output_buffer_size, bool new_table);

#endif /* HUFFMAN_CODING_H_ */
