//by Norbert Niderla, 2021

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "bitstream.h"
#include "huffman.h"

#define N_SYMBOLS (256)
#define SIZE_ENCODE_LENGTH (8)

#define ENABLE_DEBUG (0)

#define PRINT_TREE	(0)

static uint16_t lengths[N_SYMBOLS] = { 0 };
static uint32_t codewords[N_SYMBOLS] = { 0 };
#if HUFFMAN_ADAPTIVE
static uint16_t counts_enc[N_SYMBOLS] = { 0 };
static uint16_t counts_dec[N_SYMBOLS] = { 0 };
#endif

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

#if HUFFMAN_ADAPTIVE
static void huffman_tree_start(uint16_t* counts){
	for(int i = 0; i < N_SYMBOLS; i++){
		counts[i] = 1;
		lengths[i] = 1;
	}
	calc_huffman_lengths(lengths, N_SYMBOLS);
	calc_huffman_codewords(codewords, lengths, N_SYMBOLS);
}

static void huffman_update_counts(unsigned char* data, unsigned size, uint16_t* counts){
	for(unsigned i = 0; i < size; i++)
		counts[data[i]]++;

	if(counts[0] > 0x1FFF)
		for(int i = 0; i < N_SYMBOLS; i++)
			counts[i] >>= 4;		
}
#else
void huffman_set_tree_from_stream(unsigned char* data, unsigned size){
	for(int i = 0; i < N_SYMBOLS; i++){
		lengths[i] = 0;
		codewords[i] = 0;
	}
	for(unsigned i = 0; i < size; i++)
		lengths[data[i]]++;

	for(int i = 0; i < N_SYMBOLS; i++)
		if(lengths[i] == 0)
			lengths[i] = 1;

	calc_huffman_lengths(lengths, N_SYMBOLS);
	calc_huffman_codewords(codewords, lengths, N_SYMBOLS);

#if PRINT_TREE
	printf("Huffman tree:\n");
	for(int i = 0; i < N_SYMBOLS; i++){
		printf("%3d %6d %2d\n", i, codewords[i], lengths[i]);
	}
#endif
}
#endif

#if HUFFMAN_ADAPTIVE
unsigned huffman_encode(unsigned char* input, unsigned size, unsigned char* output, size_t output_buffer_size, unsigned reset_state)
#else
unsigned huffman_encode(unsigned char* input, unsigned size, unsigned char* output, size_t output_buffer_size)
#endif
{

#if HUFFMAN_ADAPTIVE
	static int huffman_tree_started = 0;
	if(huffman_tree_started == 0){
		huffman_tree_start(counts_enc);
		huffman_tree_started = 1;
	} else if (reset_state == 1){
		huffman_tree_start(counts_enc);
	} else {
		for(int i = 0; i < N_SYMBOLS; i++)
				lengths[i] = counts_enc[i];

		calc_huffman_lengths(lengths, N_SYMBOLS);
		calc_huffman_codewords(codewords, lengths, N_SYMBOLS);
	}
#endif
	
	bitstream_state_t stream_state;
	bitstream_init(&stream_state, output, output_buffer_size);
	bitstream_append_bits(&stream_state, size, SIZE_ENCODE_LENGTH);

	for(uint16_t i = 0; i < size; i++){
		bitstream_append_bits(&stream_state, codewords[input[i]], lengths[input[i]]);
	}
	bitstream_write_close(&stream_state);

#if HUFFMAN_ADAPTIVE
	huffman_update_counts(input, size, counts_enc);
#endif
	
	return stream_state.stream_used_len;
}

/* not working version of decoding
 * this one could be probably faster but while loop not cover all cases
 *
void huffman_decode(unsigned char* input, size_t input_buffer_size, unsigned char* output){
	
	bitstream_state_t stream_state;

	bitstream_init(&stream_state, input, input_buffer_size);

	unsigned long long size;
	bitstream_read_bits(&stream_state, &size, SIZE_ENCODE_LENGTH);

	for(uint16_t i = 0; i < size; i++){
		unsigned long long bit = 0;
		unsigned long long value = 0;
		uint8_t decoded = 0;
		unsigned it = 0;
		bitstream_read_bits(&stream_state, &bit, lengths[0]);
		value = bit;
		unsigned n_bits = lengths[0];
		while(decoded == 0){
			if((codewords[it] < value) & (n_bits == lengths[it]) ){
				it++;
			}
			else if(codewords[it] == value){
				decoded = 1;
				output[i] = (unsigned char)it;
				// debug 
				printf("%d\n", output[i]);
			}
			else if(codewords[it]>value){
				while(codewords[it]>value){
					bitstream_read_bits(&stream_state, &bit, 1);
					value <<= 1;
					value |= bit;
					n_bits++;
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
*/

#if HUFFMAN_ADAPTIVE
void huffman_decode(unsigned char* input, size_t input_buffer_size, unsigned char* output, unsigned reset_state)
#else
void huffman_decode(unsigned char* input, size_t input_buffer_size, unsigned char* output)
#endif
{
#if HUFFMAN_ADAPTIVE
	static int huffman_tree_started = 0;
	if(huffman_tree_started == 0){
		huffman_tree_start(counts_dec);
		huffman_tree_started = 1;
	} else if (reset_state == 1){
		huffman_tree_start(counts_dec);
	} else {
		for(int i = 0; i < N_SYMBOLS; i++)
				lengths[i] = counts_dec[i];

		calc_huffman_lengths(lengths, N_SYMBOLS);
		calc_huffman_codewords(codewords, lengths, N_SYMBOLS);
	}
#endif

	bitstream_state_t stream_state;
	bitstream_init(&stream_state, input, input_buffer_size);
	unsigned long long size;
	bitstream_read_bits(&stream_state, &size, SIZE_ENCODE_LENGTH);

	for(uint16_t i = 0; i < size; i++){
		unsigned long long bit = 0;
		unsigned long long value = 0;
		uint8_t decoded = 0;
		unsigned it = 0;
		bitstream_read_bits(&stream_state, &bit, lengths[0]);
		value = bit;
		while(decoded == 0){
			if(codewords[it] == value){
				decoded = 1;
				output[i] = (unsigned char)it;
			}else if((codewords[it] != value) & (lengths[it] == lengths[it+1])){
				it++;
			}else if((codewords[it] != value) & (lengths[it] < lengths[it+1])){
				unsigned n_bits = lengths[it+1] - lengths[it];
				bitstream_read_bits(&stream_state, &bit, n_bits);
				value <<= n_bits;
				value |= bit;
				it++;
			}
		}
	}

#if HUFFMAN_ADAPTIVE
	huffman_update_counts(output, size, counts_dec);
#endif
	
}
