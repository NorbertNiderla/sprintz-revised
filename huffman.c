//by Norbert Niderla, 2021

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include "bitstream.h"
#include "huffman.h"
#include "distribution.h"

#define N_SYMBOLS (256)
#define SIZE_ENCODE_LENGTH (8)

#define ENABLE_DEBUG (0)

uint16_t lengths[N_SYMBOLS];
uint32_t codewords[N_SYMBOLS];
uint8_t distribution_dens = 1;

static void calc_huffman_lengths(uint16_t* W, uint16_t n){
	int16_t leaf = n-1;
	int16_t root = n-1;

	//phase 1
	for(int16_t next = n-1; next>0; next--){
		if((leaf < 0) | ((root > next) & (W[root] < W[leaf]))){
			W[next] = W[root];
			W[root] = next;
			root--;
		}
		else{
			W[next] = W[leaf];
			leaf--;
		}

		if((leaf < 0) | ((root > next) & (W[root] < W[leaf]))){
			W[next] += W[root];
			W[root] = next;
			root--;
		}
		else{
			W[next] += W[leaf];
			leaf--;
		}
	}

	//phase 2
	W[1] = 0;
	for(int16_t next = 2; next < n; next++){
		W[next] = W[W[next]] + 1;
	}

	//phase 3
	int16_t avail = 1;
	int16_t used = 0;
	int16_t depth = 0;
	root = 1;
	int16_t next = 0;
	while(avail > 0 ){
		while((root < n) & (W[root] == depth)){
			used++;
			root++;
		}
		while(avail > used){
			W[next] = depth;
			next++;
			avail--;
		}

		avail = 2*used;
		depth++;
		used = 0;
	}
}

static void calc_huffman_codewords(uint32_t* codewords, uint16_t* lengths, uint16_t n){
	codewords[0] = 0;
	for(uint16_t i = 1; i < n; i++)
		codewords[i] = (codewords[i-1] + (1<<(lengths[n-1]-lengths[i-1])));

	for(uint16_t i = 0; i < n; i++)
		codewords[i] >>= (lengths[n-1]-lengths[i]);
}

int huffman_encode(unsigned char* input, int size, unsigned char* output, size_t output_buffer_size, bool new_table){
	
	if(new_table){
		distribution_dens = setFrequencies(input, size, lengths);
		calc_huffman_lengths(lengths, N_SYMBOLS);
		calc_huffman_codewords(codewords, lengths, N_SYMBOLS);
	}
	
#if ENABLE_DEBUG
	printf("Huffman table: \n");
	for(int i =0; i < N_SYMBOLS; i++)
		printf("%3d %6ld %2d\n", i, codewords[i], lengths[i]);
#endif

	int n_bytes = 0;
	bitstream_state_t stream_state;
	bitstream_init(&stream_state, output, output_buffer_size);
	n_bytes += bitstream_append(&stream_state, size, SIZE_ENCODE_LENGTH);
	n_bytes += bitstream_append(&stream_state, distribution_dens, DENS_ENCODE_BITS);

	for(uint16_t i = 0; i < size; i++)
		n_bytes += bitstream_append(&stream_state, codewords[input[i]], lengths[input[i]]);
	n_bytes += bitstream_write_close(&stream_state);
	
	return n_bytes;
}

void huffman_decode(unsigned char* input, size_t input_buffer_size, unsigned char* output){
	
	bitstream_state_t stream_state;

	bitstream_init(&stream_state, input, input_buffer_size);

	unsigned long long size;
	bitstream_read_bits(&stream_state, &size, SIZE_ENCODE_LENGTH);

	unsigned long long dens;
	bitstream_read_bits(&stream_state, &dens, DENS_ENCODE_BITS);
	
	setFrequencies_Dens(N_SYMBOLS, lengths, (uint8_t)dens);
	calc_huffman_lengths(lengths, N_SYMBOLS);
	calc_huffman_codewords(codewords, lengths, N_SYMBOLS);

	for(uint16_t i = 0; i < size; i++){
		unsigned long long bit = 0;
		unsigned long long value = 0;
		uint8_t decoded = 0;
		unsigned it = 0;
		bitstream_read_bits(&stream_state, &bit, lengths[0]);
		value = bit;
		unsigned n_bits = lengths[0];
		while(decoded == 0){
#if ENABLE_DEBUG
	printf("Act. it: %d\n", it);
#endif

			if((codewords[it] < value) & (n_bits == lengths[it]) ){
				it++;
			}
			else if(codewords[it] == value){
				decoded = 1;
				output[i] = (unsigned char)it;
			}
			else if(codewords[it]>value){
				while(codewords[it]>value){
					bitstream_read_bits(&stream_state, &bit, 1);
					value <<= 1;
					value |= bit;
					n_bits++;
#if ENABLE_DEBUG
					printf("Act. n_bits, value: %d %d \n", n_bits, (uint16_t)value);
#endif
				}
				while(lengths[it]<n_bits) it++;
			}
			else{
				while(codewords[it]>value){
					bitstream_read_bits(&stream_state, &bit, 1);
					value <<= 1;
					value |= bit;
					n_bits++;
				}
				while(lengths[it]<n_bits) it++;
			}
		}
	}
}


