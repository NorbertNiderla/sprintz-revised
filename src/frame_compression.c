/*
 * frame_compression.c
 *
 *  Created on: Sep 22, 2021
 *      Author: Norbert Niderla
 *      		Warsaw University of Technology
 */

#include "frame_compression.h"

#include "include/fire.h"
#include "include/rice.h"
#include "include/sprintz.h"
#include "include/huffman.h"
#include "include/tans.h"

#if ACK_TRANSMISSION_ENCODING
unsigned get_fire_rice_frame(unsigned char* buffer,
								const int* data,
								unsigned size_low_limit,
								unsigned size_high_limit,
								int* output_bytes,
								unsigned reset_state){
    int data_index = 0;
    int data_buffer[RICE_BATCH];

    static fire_coder_t fire_state;
	static unsigned fire_state_initialised = 0;
	if(fire_state_initialised == 0){
		fire_init(FIRE_LEARN_SHIFT, FIRE_BIT_WIDTH, &fire_state);
		fire_state_initialised = 1;
	} else if (reset_state == 1){
		fire_reset(&fire_state);
	}

    bitstream_state_t stream;
    bitstream_init(&stream, buffer, size_high_limit);

    while(stream.stream_used_len < size_low_limit){
        for(int x = 0; x < RICE_BATCH; x++)
            data_buffer[x] = data[data_index + x];
        data_index += RICE_BATCH;
        fire_encode(data_buffer, RICE_BATCH, &fire_state);
        rice_encode(data_buffer, RICE_BATCH, &stream);
    }
    bitstream_write_close(&stream);
    *output_bytes = stream.stream_used_len;
    return data_index;
}

unsigned get_sprintz_frame(unsigned char* buffer,
								const int* data,
								unsigned size_low_limit,
								unsigned size_high_limit,
								int* output_bytes,
								unsigned reset_state){
    int data_index = 0;
    int data_buffer[BITPACKING_BATCH];
    for(int x = 0; x < BITPACKING_BATCH; x++)
        data_buffer[x] = data[data_index + x];

    bitstream_state_t stream;
    bitstream_init(&stream, buffer, size_high_limit);

    sprintz_encode(data_buffer,BITPACKING_BATCH, &stream, reset_state);
    data_index += BITPACKING_BATCH;
    while(stream.stream_used_len < size_low_limit){
        for(int x = 0; x < BITPACKING_BATCH; x++)
            data_buffer[x] = data[data_index + x];
        data_index += BITPACKING_BATCH;
        sprintz_encode(data_buffer, BITPACKING_BATCH, &stream, 0);
    }
    bitstream_write_close(&stream);
    *output_bytes = stream.stream_used_len;
    return data_index;
}

unsigned get_sprintz_huffman_frame(unsigned char* buffer,
								const int* data,
								unsigned size_low_limit,
								unsigned size_high_limit,
								int* output_bytes,
                                unsigned char* output_buffer,
                                unsigned output_buffer_size,
								unsigned reset_state){
    int data_index = 0;
    int data_buffer[BITPACKING_BATCH];
    for(int x = 0; x < BITPACKING_BATCH; x++)
        data_buffer[x] = data[data_index + x];

    bitstream_state_t stream;
    bitstream_init(&stream, buffer, size_high_limit);

    sprintz_encode(data_buffer,BITPACKING_BATCH, &stream, reset_state);
    data_index += BITPACKING_BATCH;
    while(stream.stream_used_len < size_low_limit){
        for(int x = 0; x < BITPACKING_BATCH; x++)
            data_buffer[x] = data[data_index + x];
        data_index += BITPACKING_BATCH;
        sprintz_encode(data_buffer, BITPACKING_BATCH, &stream, 0);
    }
    bitstream_write_close(&stream);

    unsigned bytes = huffman_encode(buffer, stream.stream_used_len, output_buffer, output_buffer_size, reset_state);

    *output_bytes = bytes;
    return data_index;
}

unsigned get_sprintz_tans_frame(unsigned char* buffer,
								const int* data,
								unsigned size_low_limit,
								unsigned size_high_limit,
								int* output_bytes,
                                unsigned char* output_buffer,
                                unsigned output_buffer_size,
								unsigned reset_state){
    int data_index = 0;
    int data_buffer[BITPACKING_BATCH];
    for(int x = 0; x < BITPACKING_BATCH; x++)
        data_buffer[x] = data[data_index + x];

    bitstream_state_t stream;
    bitstream_init(&stream, buffer, size_high_limit);

    sprintz_encode(data_buffer,BITPACKING_BATCH, &stream, reset_state);
    data_index += BITPACKING_BATCH;
    while(stream.stream_used_len < size_low_limit){
        for(int x = 0; x < BITPACKING_BATCH; x++)
            data_buffer[x] = data[data_index + x];
        data_index += BITPACKING_BATCH;
        sprintz_encode(data_buffer, BITPACKING_BATCH, &stream, 0);
    }
    bitstream_write_close(&stream);

    unsigned bytes = tans_encode(buffer, stream.stream_used_len, output_buffer, output_buffer_size, reset_state);

    *output_bytes = bytes;
    return data_index;
}
#endif
#if SINGLE_FRAME_ENCODING
unsigned get_fire_rice_frame(unsigned char* buffer,
								const int* data,
								unsigned size_low_limit,
								unsigned size_high_limit,
								int* output_bytes){
    int data_index = 0;
    int data_buffer[RICE_BATCH];

    static fire_coder_t fire_state;
	static unsigned fire_state_initialised = 0;
	if(fire_state_initialised == 0){
		fire_init(FIRE_LEARN_SHIFT, FIRE_BIT_WIDTH, &fire_state);
		fire_state_initialised = 1;
	} else {
		fire_reset(&fire_state);
	}

    bitstream_state_t stream;
    bitstream_init(&stream, buffer, size_high_limit);

    while(stream.stream_used_len < size_low_limit){
        for(int x = 0; x < RICE_BATCH; x++)
            data_buffer[x] = data[data_index + x];
        data_index += RICE_BATCH;
        fire_encode(data_buffer, RICE_BATCH, &fire_state);
        rice_encode(data_buffer, RICE_BATCH, &stream);
    }
    bitstream_write_close(&stream);
    *output_bytes = stream.stream_used_len;
    return data_index;
}

unsigned get_sprintz_frame(unsigned char* buffer,
								const int* data,
								unsigned size_low_limit,
								unsigned size_high_limit,
								int* output_bytes){
    int data_index = 0;
    int data_buffer[BITPACKING_BATCH];
    for(int x = 0; x < BITPACKING_BATCH; x++)
        data_buffer[x] = data[data_index + x];
        
    bitstream_state_t stream;
    bitstream_init(&stream, buffer, size_high_limit);

    sprintz_encode(data_buffer,BITPACKING_BATCH, &stream, 1);
    data_index += BITPACKING_BATCH;
    while(stream.stream_used_len < size_low_limit){
        for(int x = 0; x < BITPACKING_BATCH; x++)
            data_buffer[x] = data[data_index + x];
        data_index += BITPACKING_BATCH;
        sprintz_encode(data_buffer, BITPACKING_BATCH, &stream, 0);
    }
    bitstream_write_close(&stream);
    *output_bytes = stream.stream_used_len;
    return data_index;
}

unsigned get_sprintz_huffman_frame(unsigned char* buffer,
								const int* data,
								unsigned size_low_limit,
								unsigned size_high_limit,
								int* output_bytes,
                                unsigned reset_huffman_tree,
                                unsigned char* output_buffer,
                                unsigned output_buffer_size){
    int data_index = 0;
    int data_buffer[BITPACKING_BATCH];
    for(int x = 0; x < BITPACKING_BATCH; x++)
        data_buffer[x] = data[data_index + x];
        
    bitstream_state_t stream;
    bitstream_init(&stream, buffer, size_high_limit);

    sprintz_encode(data_buffer,BITPACKING_BATCH, &stream, 1);
    data_index += BITPACKING_BATCH;
    while(stream.stream_used_len < size_low_limit){
        for(int x = 0; x < BITPACKING_BATCH; x++)
            data_buffer[x] = data[data_index + x];
        data_index += BITPACKING_BATCH;
        sprintz_encode(data_buffer, BITPACKING_BATCH, &stream, 0);
    }
    bitstream_write_close(&stream);

    if(reset_huffman_tree){
        huffman_set_tree_from_stream(buffer, stream.stream_used_len);
    }
    unsigned bytes = huffman_encode(buffer, stream.stream_used_len, output_buffer, output_buffer_size);

    *output_bytes = bytes;
    return data_index;
}

unsigned get_sprintz_tans_frame(unsigned char* buffer,
								const int* data,
								unsigned size_low_limit,
								unsigned size_high_limit,
								int* output_bytes,
                                unsigned char* output_buffer,
                                unsigned output_buffer_size){
    int data_index = 0;
    int data_buffer[BITPACKING_BATCH];
    for(int x = 0; x < BITPACKING_BATCH; x++)
        data_buffer[x] = data[data_index + x];
        
    bitstream_state_t stream;
    bitstream_init(&stream, buffer, size_high_limit);

    sprintz_encode(data_buffer,BITPACKING_BATCH, &stream, 1);
    data_index += BITPACKING_BATCH;
    while(stream.stream_used_len < size_low_limit){
        for(int x = 0; x < BITPACKING_BATCH; x++)
            data_buffer[x] = data[data_index + x];
        data_index += BITPACKING_BATCH;
        sprintz_encode(data_buffer, BITPACKING_BATCH, &stream, 0);
    }
    bitstream_write_close(&stream);

    unsigned bytes = tans_encode(buffer, stream.stream_used_len, output_buffer, output_buffer_size);

    *output_bytes = bytes;
    return data_index;
}
#endif
