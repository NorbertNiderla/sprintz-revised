//by Norbert Niderla, 2021

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

#include "bitstream.h"
#include "tans.h"
#include "distribution.h"

#define TANS_MODE (0)

#if TANS_MODE
	#define L (2048)
	#define TABLE_BUILDING_L (1280)
	#define TANS_STATE_BITS_ENCODE 	(12)
#else
	#define L (512)
	#define TABLE_BUILDING_L (340)
	#define TANS_STATE_BITS_ENCODE 	(10)
#endif

#define SYMBOLS 256
#define ENABLE_REAL_BITS_ADDING (1)

#define ENABLE_DEBUG	(0)
#define PRINT_OCC		(0)
#define PRINT_TABLE		(0)
#define ENABLE_TOO_LONG_SYMBOLS_SUB (1)

#define CLEAR_VALUE(val, x) (val&(~(0xFFFFFFFF<<x)))

static uint16_t occ[SYMBOLS] = { 0 };
static uint16_t offset[SYMBOLS] = { 0 };
static uint16_t output_states[L] = { 0 };
static uint16_t tans_state = L;
static uint16_t tans_dec_state = L;

void buildTable(void) {

#if PRINT_OCC
	printf("tans occ:\n");
	for(int i=0;i<SYMBOLS;i++) printf("%3d %4d\n", i, occ[i]);
#endif

	int current_state = 0;
	int current_output_state = L;

	offset[0] = 0;
	for (int i = 1; i < SYMBOLS; i++)
		offset[i] = offset[i - 1] + occ[i-1];

	for (int i = 0; i < L; i++) {
		output_states[current_state] = current_output_state++;
		current_state = (current_state + TABLE_BUILDING_L + 3) % L;
	}

#if PRINT_TABLE
	printf("tANS OFFSET TABLE:\n");
	for (int i = 0; i < SYMBOLS; i++)
		printf("%3d %3d\n", i, offset[i]);
	printf("tANS OUTPUT STATES:\n");
	for (int i = 0; i < L; i++)
		printf("%3d %3d\n", i, output_states[i]);
#endif
}

static uint16_t get_output_state(uint8_t symbol, uint16_t input_state) {
	int idx = offset[symbol] + input_state - occ[symbol];
	return (output_states[idx]);
}

int tansEncode(int *n, int size, unsigned char *output,
		size_t output_buffer_size, bool new_table, int method) {
	if (new_table) {
		switch (method) {
		case 0:
			setOccurrences(n, size, L, occ);
			break;
		case 1:
			setOccurrencesNormal(n, size, L, occ);
			break;
		default:
			break;
		}
		buildTable();
	}

	bitstream_state_t state;
	bitstream_state_t *state_p = &state;
	bitstream_init(state_p, &output[2], output_buffer_size - 2);
	uint32_t I_s = tans_state;
	uint32_t I_s_max;
	int bits = 0;

	for (int i = 0; i < size; i++) {
#if ENABLE_TOO_LONG_SYMBOLS_SUB
		if ((n[i]) >= SYMBOLS) {
#if ENABLE_DEBUG
			printf("tANS encoding - Symbol too big: %d\n", n[i]);
#endif
			n[i] = 0;
#if ENABLE_REAL_BITS_ADDING
			bits += 16;
#endif
		}
#endif
		I_s_max = (occ[n[i]] << 1) - 1;
		while(I_s>I_s_max){
			bitstream_append_bit(state_p,(unsigned long long)I_s);
			bits++;
			I_s >>=1;
		}

		I_s = get_output_state(n[i], I_s);
#if ENABLE_DEBUG
		printf("tANS encoding - SYMBOL:%3d, BITS:%2ld, STATE:%3d\n", symbol, nBits, tans_state);
#endif
	}
	bitstream_write_close(state_p);
	tans_state = I_s;

	memcpy(output, &tans_state, sizeof(uint16_t));
#if ENABLE_REAL_BITS_ADDING
	bits += TANS_STATE_BITS_ENCODE;
	bits += DENS_ENCODE_BITS;
#endif
	return bits;
}

int tansEncode_char(unsigned char *n, int size, unsigned char *output, size_t output_buffer_size, bool new_table, int method) {
	if (new_table) {
		switch (method) {
		case 0:
			setOccurrences_char(n, size, L, occ);
			break;
		case 1:
			setOccurrencesNormal_char(n, size, L, occ);
			break;
		default:
			break;
		}
		buildTable();
	}

	bitstream_state_t state;
	bitstream_state_t *state_p = &state;
	bitstream_init(state_p, &output[2], output_buffer_size - 2);
	uint32_t I_s = tans_state;
	uint32_t I_s_max;
	int bits = 0;

	for (int i = 0; i < size; i++) {
		I_s_max = (occ[n[i]] << 1) - 1;
		while(I_s>I_s_max){
			bitstream_append_bit(state_p,(unsigned long long)I_s);
			bits++;
			I_s >>=1;
		}

		I_s = get_output_state(n[i], I_s);
#if ENABLE_DEBUG
		printf("tANS encoding - SYMBOL:%3d, BITS:%2ld, STATE:%3d\n", symbol, nBits, tans_state);
#endif
	}
	bitstream_write_close(state_p);
	tans_state = I_s;

	memcpy(output, &tans_state, sizeof(uint16_t));
#if ENABLE_REAL_BITS_ADDING
	bits += TANS_STATE_BITS_ENCODE;
	bits += DENS_ENCODE_BITS;
#endif
	return bits;
}


static int getIdxFromState(int state) {
	int idx = 0;
	while (idx < L)
		if (output_states[idx++] == state)
			return idx - 1;
	return (-1);
}

static int getSymbolFromOffset(int val) {
	int idx = 0;
	while (idx < SYMBOLS) {
		if (((offset[idx] <= val) & (val < offset[idx+1])))
			return idx;
		idx++;
	}
	return (-1);
}

void tansDecode(unsigned char *input, size_t output_buffer_size, int bits,
		int size, int *n) {
	bitstream_state_t state;
	bitstream_state_t *state_p = &state;
	int bytes = (bits - bits % 8) / 8;
	bitstream_init_from_behind(state_p, &input[bytes + 2],
			output_buffer_size - 2, bits % 8);
	memcpy(&tans_dec_state, input, sizeof(uint16_t));

	int min_new_state = L;
	int n_idx = size - 1;
	int table_idx;
	int nBits = 1;
	unsigned long long new_bits = 0;
	int symbol = 0;

	while (bits > 0) {
		table_idx = getIdxFromState(tans_dec_state);
		symbol = getSymbolFromOffset(table_idx);
		if (symbol == (-1)) {
			printf("tansDecode: getSymbolFromOffset: error, wrong symbol");
			symbol = 0;
		}
		n[n_idx--] = symbol;
		tans_dec_state = table_idx - offset[symbol] + occ[symbol];
		while (tans_dec_state < min_new_state) {
			new_bits = 0;
			bitstream_read_bits_from_behind(state_p, &new_bits, nBits);
			bits -= nBits;
			tans_dec_state <<= nBits;
			tans_dec_state += new_bits;
		}
	}
}

