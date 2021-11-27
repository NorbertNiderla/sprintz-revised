//by Norbert Niderla, 2021

#include <stdint.h>
#include <stdlib.h>
#include <math.h>

#include "bitstream.h"
#include "tans.h"

#define L 				(512)
#define L_BITS 			(9)
#define DENS_BITS		(5) //this must be lower or equal to 8 bits
#define DENS_MASK	(0xFFFFFFFF<<(L_BITS-DENS_BITS))
#define TABLE_BUILDING_L (340)

#define SYMBOLS 256

#define ENABLE_DEBUG	(0)
#include "debug.h"
#define PRINT_OCC		(0)
#define PRINT_TABLE		(1)


static void tans_build_table(uint16_t* occ, uint16_t* offset, uint16_t* output_states) {
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
	DEBUG("tANS OFFSET TABLE:\n");
	for (int i = 0; i < SYMBOLS; i++)
		DEBUG("%3d %3d %3d\n", i, occ[i], offset[i]);
	/*
	DEBUG("tANS OUTPUT STATES:\n");
	for (int i = 0; i < L; i++)
		DEBUG("%3d %3d\n", i, output_states[i]);
	*/
#endif
}

#if TANS_ADAPTIVE
static void tans_tree_start(uint16_t* occ, uint16_t* offset, uint16_t* output_states){
	int count_left = L;
	int idx = 0;
	while(count_left > 0){
		occ[idx]++;
		idx++;
		count_left--;
		if(idx == SYMBOLS) idx = 0;
	}
	tans_build_table(occ, offset, output_states);
}

static void tans_update_tree(unsigned char* data, int size, uint8_t* subtr_idx, uint16_t* occ, uint16_t* offset, uint16_t* output_states){
	uint8_t sidx = *subtr_idx;
	for(int i = 0; i<size;i++){
		occ[data[i]]++;
	}
	int i = 0;
	while( i < size ){
		if(occ[sidx] != 1){
			occ[sidx]--;
			i++;
		}
		sidx++;
	}
	*subtr_idx = sidx;
	tans_build_table(occ, offset, output_states);
}
#else
static unsigned set_occurences_from_stream(unsigned char* input, int size, uint16_t* occ) {
	int cnt = 0;
	for (int i = 0; i < size; i++) if (input[i] == 0) cnt++;
	float dens = ((float) cnt / (float) size);

	if (dens == 1)
		occ[0] = L - SYMBOLS + 1;
	else if (dens == 0)
		occ[0] = 1;
	else
		occ[0] = ((int) roundf(dens * (float) L)) & DENS_MASK;

	for (int i = 1; i < SYMBOLS; i++)
		occ[i] = 1;

	int missing_bits = L - occ[0] - SYMBOLS + 1;
	int idx = 0;

	if (missing_bits > 0) {
		while (missing_bits != 0) {
			occ[idx++]++;
			missing_bits--;
			if (idx == SYMBOLS)
				idx = 0;
		}
	} else if (missing_bits < 0) {
		occ[0] += missing_bits;
	}
	return occ[0]>>(L_BITS-DENS_BITS);
}

static void set_occurences_from_dens(int occ_0, uint16_t* occ){
	
	occ[0] = occ_0;

	for (int i = 1; i < SYMBOLS; i++)
		occ[i] = 1;

	int missing_bits = L - occ[0] - SYMBOLS + 1;
	int idx = 0;

	if (missing_bits > 0) {
		while (missing_bits != 0) {
			occ[idx++]++;
			missing_bits--;
			if (idx == SYMBOLS)
				idx = 0;
		}
	} else if (missing_bits < 0) {
		occ[0] += missing_bits;
	}
}
#endif

#if TANS_ENCODER_ON
static uint16_t occ_enc[SYMBOLS] = { 0 };
static uint16_t offset_enc[SYMBOLS] = { 0 };
static uint16_t output_states_enc[L] = { 0 };
static uint16_t tans_state_enc = L;

static uint16_t get_output_state(uint8_t symbol, uint16_t input_state) {
	int idx = offset_enc[symbol] + input_state - occ_enc[symbol];
	return (output_states_enc[idx]);
}
/*
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
*/

#if TANS_ADAPTIVE
unsigned tans_encode(unsigned char* data, int size, unsigned char *output, size_t output_buffer_size, unsigned reset_state)
#else
unsigned tans_encode(unsigned char* data, int size, unsigned char *output, size_t output_buffer_size)
#endif
{
#if TANS_ADAPTIVE
	static int tans_tree_started = 0;
	if(tans_tree_started == 0){
		tans_tree_start(occ_enc, offset_enc, output_states_enc);
		tans_tree_started = 1;
	} else if(reset_state == 1){
		tans_tree_start(occ_enc, offset_enc, output_states_enc);
	}
#else
	unsigned dens = set_occurences_from_stream(data, size, occ_enc);
	tans_build_table(occ_enc, offset_enc, output_states_enc);
#endif
	bitstream_state_t state;
	bitstream_init(&state, output, output_buffer_size);
	uint32_t I_s = tans_state_enc;
	uint32_t I_s_max;

	for (int i = 0; i < size; i++) {
		I_s_max = (occ_enc[data[i]] << 1) - 1;
		while(I_s>I_s_max){
			bitstream_append(&state,(unsigned long long)(I_s&1), 1);
			I_s >>=1;
		}
		I_s = get_output_state(data[i], I_s);
		/* DEBUG("tans_encoding - SYMBOL:%3d, STATE:%3d\n", data[i], I_s);
		*/
	}
	tans_state_enc = I_s;
	bitstream_append_bits(&state, tans_state_enc, 8*sizeof(uint16_t));
#if !TANS_ADAPTIVE
	bitstream_append_bits(&state, dens, DENS_BITS);
#endif
	bitstream_append(&state, 1, 1);
	bitstream_write_close(&state);

#if TANS_ADAPTIVE
	static uint8_t subtraction_idx = 0;
	tans_update_tree(data, size, &subtraction_idx, occ_enc, offset_enc, output_states_enc);
#endif

	return state.stream_used_len;
}
#endif

#if TANS_DECODER_ON
static uint16_t tans_dec_state = L;
static uint16_t symbol_map[L] = {0};
static uint16_t occ_dec[SYMBOLS] = { 0 };
static uint16_t offset_dec[SYMBOLS] = { 0 };
static uint16_t output_states_dec[L] = { 0 };


static void tans_build_symbol_map(void){
	int occ_counter = 0;
		int act_symbol = 0;
		for (int i = 0; i < L; i++) {
			symbol_map[i] = act_symbol;
			occ_counter++;
			if(occ_counter==occ_dec[act_symbol]){
				act_symbol++;
				occ_counter = 0;
			}
		}
}

static int get_idx_from_state(int state) {
	int idx = 0;
	while (idx < L)
		if (output_states_dec[idx++] == state)
			return idx - 1;
	return (-1);
}

#if !TANS_ADAPTIVE
static void tans_build_table_from_stream_decoding(void) {

	int current_state = 0;
	int current_output_state = L;

	offset[0] = 0;
	for (int i = 1; i < SYMBOLS; i++)s
		offset[i] = offset[i - 1] + occ[i-1];

	for (int i = 0; i < L; i++) {
		output_states[current_state] = current_output_state++;
		current_state = (current_state + TABLE_BUILDING_L + 3) % L;
	}

	int occ_counter = 0;
	int act_symbol = 0;
	for (int i = 0; i < L; i++) {
		symbol_map[i] = act_symbol;
		occ_counter++;
		if(occ_counter==occ[act_symbol]){
			act_symbol++;
			occ_counter = 0;
		}
	}
}
#endif
void tans_decode(unsigned char *input, unsigned bytes, unsigned char* data) {
#if TANS_ADAPTIVE
	static int tans_tree_started = 0;
	if(tans_tree_started == 0){
		tans_tree_start(occ_dec, offset_dec, output_states_dec);
		tans_build_symbol_map();
		tans_tree_started = 1;
	}
#endif
	
	bitstream_state_t state;
	int bits = bytes << 3;

	int bit = 0;
	int x = 0;
	while((bit == 0) & (x < 8)){
		bit = (input[bytes-1]>>x)&1;
		x++;
	}
	bitstream_init_from_behind(&state, &input[bytes-1], bytes, 8 - x);
	bits -= x;
#if !TANS_ADAPTIVE
	unsigned long long dens;
	bitstream_read_bits_from_behind(&state, &dens, DENS_BITS);
	bits -= DENS_BITS;
	set_occurences_from_dens(dens << (L_BITS - DENS_BITS));
	tans_build_table_from_stream_decoding();
#endif
	unsigned long long temp;
	bitstream_read_bits_from_behind(&state, &temp, 16);
	bits -= 16;
	tans_dec_state = temp;

	int min_new_state = L;
	int n_idx = 0;
	int table_idx;
	int nBits = 1;
	unsigned long long new_bits = 0;
	int symbol = 0;

	while (bits > 0) {
		table_idx = get_idx_from_state(tans_dec_state);
		symbol = symbol_map[table_idx];
		if (symbol == (-1)) {
			DEBUG("tans_decode: get_symbol_from_offset: error, wrong symbol");
			symbol = 0;
		}
		DEBUG("tans_decoding - SYMBOL:%3d, STATE:%3d\n", symbol, tans_dec_state);
		data[n_idx++] = symbol;
		tans_dec_state = table_idx - offset_dec[symbol] + occ_dec[symbol];
		while (tans_dec_state < min_new_state) {
			new_bits = 0;
			bitstream_read_bits_from_behind(&state, &new_bits, nBits);
			bits -= nBits;
			tans_dec_state <<= nBits;
			tans_dec_state += new_bits;
		}
	}

	for(int i = 0, k = n_idx-1; i<=((n_idx-1)>>1);i++,k--){
		unsigned char temp = data[i];
		data[i] = data[k];
		data[k] = temp;
	}

#if TANS_ADAPTIVE
	static uint8_t subtraction_idx = 0;
	tans_update_tree(data, n_idx, &subtraction_idx, occ_dec, offset_dec, output_states_dec);
	tans_build_symbol_map();
#endif
}

#endif
