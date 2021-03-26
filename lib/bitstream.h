/*
 * bitstream.h
 *
 *  Created on: 2 lis 2018
 *      Author: fs
 *
 * 13 sie 2020: reading from behind by Norbert Niderla
 * 
 * 
 */

#ifndef SYS_CODER_INCLUDE_BITSTREAM_H_
#define SYS_CODER_INCLUDE_BITSTREAM_H_

#include <stddef.h>
#include <stdint.h>

typedef struct {
    unsigned char processing_byte_buffer;
    unsigned bit_count_in_buffer;

    unsigned char* stream_ptr_last;
    unsigned int stream_free_len, stream_used_len;
    unsigned char* stream_ptr_first;
} bitstream_state_t;

void bitstream_init(bitstream_state_t* state, unsigned char* stream, size_t stream_max_len);
void bitstream_init_from_behind(bitstream_state_t* state, unsigned char* stream, size_t stream_max_len, int bit_count_in_last_byte);
void bitstream_reset(bitstream_state_t* state, unsigned char* stream, size_t stream_max_len);

// WRITE FUNCTIONS
unsigned int bitstream_append_bits(bitstream_state_t* state, unsigned long long value, unsigned n_bits_value);
int bitstream_append_int32(bitstream_state_t* state, int32_t value);
int bitstream_append_int16(bitstream_state_t* state, int16_t value);
int bitstream_append_int8(bitstream_state_t* state, int8_t value);
int bitstream_write_close(bitstream_state_t* state);

// READ FUNCTIONS
unsigned int bitstream_read_bits(bitstream_state_t* state, unsigned long long *value, unsigned n_bits_value);
unsigned int bitstream_read_bits_from_behind(bitstream_state_t* state, unsigned long long *value, unsigned n_bits_value);
unsigned int bitstream_read_bits_int(bitstream_state_t* state, int *value, unsigned n_bits_value);
int bitstream_read_int32(bitstream_state_t* state, int32_t *value);
int bitstream_read_int16(bitstream_state_t* state, int16_t *value);
int bitstream_read_int8(bitstream_state_t* state, int8_t *value);
int bitstream_read_close(bitstream_state_t* state);
int bitstream_read_panning_bits(bitstream_state_t* state);

#endif /* SYS_CODER_INCLUDE_BITSTREAM_H_ */
