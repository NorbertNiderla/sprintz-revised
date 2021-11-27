#include <assert.h>
#include <math.h>
#include <string.h>
#include <stdio.h>

#include "bitstream.h"

void bitstream_init(bitstream_state_t* state, unsigned char* stream, size_t stream_max_len) {
    state->processing_byte_buffer = 0x00;
    state->bit_count_in_buffer = 0;
    state->stream_used_len = 0;
    state->stream_ptr_last = stream;
    state->stream_free_len = stream_max_len;
}

void bitstream_init_from_behind(bitstream_state_t* state, unsigned char* stream, size_t stream_max_len, int bit_count_in_last_byte) {
    state->processing_byte_buffer = *stream;
    state->bit_count_in_buffer = bit_count_in_last_byte;
    state->stream_used_len = 0;
    state->stream_ptr_last = --stream;
    state->stream_free_len = stream_max_len;
}

void bitstream_reset(bitstream_state_t* state, unsigned char* stream, size_t stream_max_len) {
    state->stream_used_len = 0;
    state->stream_ptr_last = stream;
    state->stream_free_len = stream_max_len;
}

// WRITE FUNCTIONS

/*
 * Bytes written in the stream
 */
int bitstream_append_bits(bitstream_state_t* state, unsigned long long value, unsigned n_bits_value) {
    unsigned int out = 0;
    unsigned curr_n_bits = 0;
    while (n_bits_value > 0) {
        if(n_bits_value < (8 - state->bit_count_in_buffer))
            curr_n_bits = n_bits_value;
        else
            curr_n_bits = 8 - state->bit_count_in_buffer;    
        unsigned long long itValue = value;
        if (curr_n_bits != n_bits_value) {
            itValue = value >> (n_bits_value - curr_n_bits);
            value = value - (itValue << (n_bits_value - curr_n_bits));
        }
        state->processing_byte_buffer = (state->processing_byte_buffer << curr_n_bits); // Making space for new content
        state->processing_byte_buffer = state->processing_byte_buffer | (itValue & ((2 << curr_n_bits) - 1)); // Adding the new content
        state->bit_count_in_buffer += curr_n_bits;
        // if full, write to buffer and clear
        if (state->bit_count_in_buffer == 8) {
            //logPrint(LOG_VERBOSE, "Byte 0x%X output to stream", processingByte);
            state->stream_ptr_last[0] = state->processing_byte_buffer;
            state->stream_ptr_last++;
            state->stream_used_len++;
            state->stream_free_len--;
            out++;
            if(state->stream_free_len == 0){
            	printf("Bitstream out of space!\n");
                return -1;
            }
            state->processing_byte_buffer = 0x00;
            state->bit_count_in_buffer = 0;
        }
        n_bits_value -= curr_n_bits;
    }

    return out;
}

int bitstream_append_bit(bitstream_state_t* state, unsigned long long value) {

        state->processing_byte_buffer <<= 1; // Making space for new content
        state->processing_byte_buffer |= (value & 1); // Adding the new content
        state->bit_count_in_buffer ++;
        if (state->bit_count_in_buffer == 8) {
            state->stream_ptr_last[0] = state->processing_byte_buffer;
            state->stream_ptr_last++;
            state->stream_used_len++;
            state->stream_free_len--;
            if(state->stream_free_len == 0){
                return -1;
            }
            state->processing_byte_buffer = 0x00;
            state->bit_count_in_buffer = 0;
            return 1;
        }
        return 0;
}

unsigned int bitstream_append(bitstream_state_t* state, unsigned long long value, int n_bits_value){
	int n_bytes = 0;
	while((n_bits_value--)>0){
		state->processing_byte_buffer <<= 1;
		state->processing_byte_buffer |= (value & 1);
		value >>= 1;
		state->bit_count_in_buffer ++;
		if (state->bit_count_in_buffer == 8) {
			state->stream_ptr_last[0] = state->processing_byte_buffer;
			state->stream_ptr_last++;
			state->stream_used_len++;
			state->stream_free_len--;
			state->processing_byte_buffer = 0x00;
			state->bit_count_in_buffer = 0;
			n_bytes++;
			if(state->stream_free_len == 0){
				printf("Bitstream out of space\n");
                return -1;
			}
		}
	}
	return n_bytes;
}


int bitstream_append_int32(bitstream_state_t* state, int32_t value) {
    assert(state->stream_free_len > 0);

    int size = bitstream_append_bits(state, value, 32);

    return size; // Number of bytes written
}

int bitstream_append_int16(bitstream_state_t* state, int16_t value) {
    assert(state->stream_free_len > 0);

    int size = bitstream_append_bits(state, value, 16);

    return size; // Number of bytes written
}

int bitstream_append_int8(bitstream_state_t* state, int8_t value) {
    assert(state->stream_free_len > 0);

    int size = bitstream_append_bits(state, value, 8);
    return size; // Number of bytes written
}

int bitstream_write_close(bitstream_state_t* state){
    uint8_t panning_bits = 8 - state->bit_count_in_buffer;
    if(panning_bits)
        return bitstream_append_bits(state, 0ULL, panning_bits);

    return 0;
}

// READ FUNCTIONS

unsigned int bitstream_read_bits(bitstream_state_t* state, unsigned long long *value, unsigned n_bits_value){
    unsigned int out = 0;
    unsigned long long _value = 0ULL;
    while(n_bits_value > 0){
        if(state->bit_count_in_buffer == 0){
            if(state->stream_free_len == 0)
                return -1;

            state->processing_byte_buffer = state->stream_ptr_last[0];
            state->bit_count_in_buffer = 8;
            state->stream_ptr_last++;
            state->stream_free_len--;
            out++;
        }
        unsigned short curr_n_bits = fmin(n_bits_value, state->bit_count_in_buffer);
        _value <<= curr_n_bits;
        uint8_t byte_buffer_copy = state->processing_byte_buffer;
        byte_buffer_copy >>= 8 - curr_n_bits;
        _value |= byte_buffer_copy;
        state->processing_byte_buffer <<= curr_n_bits;
        state->bit_count_in_buffer -= curr_n_bits;
        n_bits_value -= curr_n_bits;
    }

    //if(out >= 0)
        *value = _value;

    return out;
}

unsigned int bitstream_shift_read_bits(bitstream_state_t* state, unsigned long long *value, unsigned n_bits_value){
    unsigned int out = 0;
    while(n_bits_value > 0){
        if(state->bit_count_in_buffer == 0){
            if(state->stream_free_len == 0)
                return -1;

            state->processing_byte_buffer = state->stream_ptr_last[0];
            state->bit_count_in_buffer = 8;
            state->stream_ptr_last++;
            state->stream_free_len--;
            out++;
        }
        unsigned short curr_n_bits = fmin(n_bits_value, state->bit_count_in_buffer);
        *value <<= curr_n_bits;
        uint8_t byte_buffer_copy = state->processing_byte_buffer;
        byte_buffer_copy >>= 8 - curr_n_bits;
        *value |= byte_buffer_copy;
        state->processing_byte_buffer <<= curr_n_bits;
        state->bit_count_in_buffer -= curr_n_bits;
        n_bits_value -= curr_n_bits;
    }

    return out;
}

unsigned int bitstream_read_bits_from_behind(bitstream_state_t* state, unsigned long long *value, unsigned n_bits_value){
    unsigned int out = 0;
    unsigned long long _value = 0ULL;
    unsigned n_bits_value_c = n_bits_value;
    while(n_bits_value > 0){
        if(state->bit_count_in_buffer == 0){
            if(state->stream_free_len == 0)
                return -1;

            state->processing_byte_buffer = state->stream_ptr_last[0];
            state->bit_count_in_buffer = 8;
            state->stream_ptr_last--;
            state->stream_free_len--;
            out++;
        }
        unsigned short curr_n_bits = fmin(n_bits_value, state->bit_count_in_buffer);
        _value >>= curr_n_bits;
        unsigned long long byte_buffer_copy = state->processing_byte_buffer;
        byte_buffer_copy <<= state->bit_count_in_buffer-curr_n_bits+56;
        _value |= byte_buffer_copy;
        //state->processing_byte_buffer <<= curr_n_bits;
        state->bit_count_in_buffer -= curr_n_bits;
        n_bits_value -= curr_n_bits;
    }

    //if(out >= 0)
        *value = (_value>>(64-n_bits_value_c));

    return out;
}

unsigned int bitstream_read_bits_int(bitstream_state_t* state, int *value, unsigned n_bits_value)
{
    unsigned int out = 0;
    unsigned long long _value = 0ULL;
    while(n_bits_value > 0){
        if(state->bit_count_in_buffer == 0){
            if(state->stream_free_len == 0)
                return -1;

            state->processing_byte_buffer = state->stream_ptr_last[0];
            state->bit_count_in_buffer = 8;
            state->stream_ptr_last++;
            state->stream_free_len--;
            out++;
        }
        unsigned short curr_n_bits = fmin(n_bits_value, state->bit_count_in_buffer);
        _value <<= curr_n_bits;
        uint8_t byte_buffer_copy = state->processing_byte_buffer;
        byte_buffer_copy >>= 8 - curr_n_bits;
        _value |= byte_buffer_copy;
        state->processing_byte_buffer <<= curr_n_bits;
        state->bit_count_in_buffer -= curr_n_bits;
        n_bits_value -= curr_n_bits;
    }

    //if(out >= 0)
        *value = (int)_value;

    return out;
}

int bitstream_read_int32(bitstream_state_t* state, int32_t *value){
    assert(state->stream_free_len > 0);

    const size_t size = sizeof(int32_t);

    if (state->stream_free_len < size)
        return -1; // Meaning, I didn't encode anything there.. I need 4 bytes

    memcpy(value, state->stream_ptr_last, size);

    state->stream_ptr_last += size;
    state->stream_free_len -= size;

    return size; // Number of bytes written
}

int bitstream_read_int16(bitstream_state_t* state, int16_t *value){
    assert(state->stream_free_len > 0);

    const size_t size = sizeof(int16_t);

    if (state->stream_free_len < size)
        return -1; // Meaning, I didn't encode anything there.. I need 2 bytes

    memcpy(value, state->stream_ptr_last, size);

    state->stream_ptr_last += size;
    state->stream_free_len -= size;

    return size; // Number of bytes written
}

int bitstream_read_int8(bitstream_state_t* state, int8_t *value){
    assert(state->stream_free_len > 0);

    const size_t size = sizeof(int8_t);

    if (state->stream_free_len < size)
        return -1; // Meaning, I didn't encode anything there.. I need 1 bytes

    memcpy(value, state->stream_ptr_last, size);

    state->stream_ptr_last += size;
    state->stream_free_len -= size;

    return size; // Number of bytes written
}

int bitstream_read_close(bitstream_state_t* state){
    return state->bit_count_in_buffer + 8*state->stream_free_len;
}

int bitstream_read_panning_bits(bitstream_state_t* state){
	unsigned long long temp;
	bitstream_read_bits(state,&temp,state->bit_count_in_buffer);
	return state->bit_count_in_buffer;
}
