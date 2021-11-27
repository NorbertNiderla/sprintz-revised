#include <stdlib.h>
#include <assert.h>

#include "sprintz.h"
#include "bitstream.h"
#include "fire.h"
#include "tans.h"
#include "huffman.h"


#define SB_ENCODING_BITS	(5)

static void zigzag_encode(int* data, int size){
	for(int i = 0; i < size; i++)
		data[i] = (data[i] >> 31) ^ (data[i] << 1);
}

static void zigzag_decode(int* data, int size){
	for(int i = 0; i < size; i++)
		data[i] = (data[i] >> 1) ^ -(data[i] & 1);
}

static unsigned count_significant_bits(int* data, int size)
{
	int sb = 0;
	int temp = 32;
	for (int idx = 0; idx < size; idx++){
		if(data[idx]==0) temp = 0;
		else temp = 32 - __builtin_clz(data[idx]);
		if(temp > sb) sb = temp;
	}
	return sb;
}

static void bitpacking_pack(int* data, unsigned size, bitstream_state_t* stream){
	div_t loops = div(size, BITPACKING_BATCH);
	assert(loops.rem == 0); /*size must be dividable by BITPACKING_BATCH */

	for(int i = 0; i < loops.quot; i++){
		unsigned bits = count_significant_bits(&data[i*BITPACKING_BATCH], BITPACKING_BATCH);
		bitstream_append_bits(stream, bits, SB_ENCODING_BITS);
		for(int x = 0; x < BITPACKING_BATCH; x++){
			bitstream_append_bits(stream, data[i*BITPACKING_BATCH + x], bits);
		}
	}
}

static unsigned bitpacking_unpack(bitstream_state_t* stream, unsigned size, int* data){
	unsigned samples_read = 0;
	unsigned read_bytes = 0;
	unsigned long long bits;
	unsigned data_idx = 0;
	while ( read_bytes < size ){
		read_bytes += bitstream_read_bits(stream, &bits, SB_ENCODING_BITS);
		for(int x = 0; x < BITPACKING_BATCH; x++){
			read_bytes += bitstream_read_bits_int(stream, &data[data_idx], bits);
			data_idx++;
		}
		samples_read += BITPACKING_BATCH;
	}
	return samples_read;
}

void sprintz_encode(int* data, int size, bitstream_state_t* stream, unsigned reset_state){
	static fire_coder_t fire_state;
	static unsigned fire_state_initialised = 0;
	if(fire_state_initialised == 0){
		fire_init(FIRE_LEARN_SHIFT, FIRE_BIT_WIDTH, &fire_state);
		fire_state_initialised = 1;
	} else if(reset_state == 1){
		fire_reset(&fire_state);
	}

	fire_encode(data, size, &fire_state);
	zigzag_encode(data, size);
	bitpacking_pack(data, size, stream);
}

unsigned sprintz_decode(bitstream_state_t* stream, unsigned size, int* data, unsigned reset_state){
	static fire_coder_t fire_state;
	static unsigned fire_state_initialised = 0;
	if(fire_state_initialised == 0){
		fire_init(FIRE_LEARN_SHIFT, FIRE_BIT_WIDTH, &fire_state);
		fire_state_initialised = 1;
	} else if(reset_state == 1){
		fire_reset(&fire_state);
	}
	
	unsigned samples = bitpacking_unpack(stream, size, data);
	zigzag_decode(data, samples);
	fire_decode(data, samples, &fire_state);

	return samples;
}