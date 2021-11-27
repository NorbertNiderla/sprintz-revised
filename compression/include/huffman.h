/*
 * huffman_coding.h
 *
 *  Created on: May 5, 2021
 *      Author: Norbert Niderla
 */

#ifndef HUFFMAN_CODING_H_
#define HUFFMAN_CODING_H_

#define HUFFMAN_ADAPTIVE    (1)

#if !HUFFMAN_ADAPTIVE
void huffman_set_tree_from_stream(unsigned char* data, unsigned size);
unsigned huffman_encode(unsigned char* input, unsigned size, unsigned char* output, size_t output_buffer_size);
void huffman_decode(unsigned char* input, size_t input_buffer_size, unsigned char* output);
#else
unsigned huffman_encode(unsigned char* input, unsigned size, unsigned char* output, size_t output_buffer_size, unsigned reset_state);
void huffman_decode(unsigned char* input, size_t input_buffer_size, unsigned char* output, unsigned reset_state);
#endif
#endif /* HUFFMAN_CODING_H_ */
