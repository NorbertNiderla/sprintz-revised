#include <stdint.h>
#include <stdlib.h>
#include <math.h>
#include "bitstream.h"
#include "tans.h"
#include "distribution.h"

#define SYMBOLS 256
#define L 2048

static int table_parameter = 0;

int GetTableParameter(void){return table_parameter;}

typedef struct table_el_t{
	uint8_t symbol;
	uint8_t input_state;
	uint16_t output_state;
}table_el;

static uint16_t occ[SYMBOLS] = {0};
static int offset[SYMBOLS];
static table_el table[L];
static int tans_state = L;
static int tans_dec_state = L;
static int b = 2;
static uint16_t I_s[SYMBOLS] = {0};
static uint16_t I[L] = {0};

void buildTable(void){
	offset[0] = 0;
	for(int i = 1;i<SYMBOLS;i++) offset[i]=offset[i-1]+occ[i-1];
	for(int i = 0;i<SYMBOLS;i++) I_s[i] = occ[i];
	for(int i = 0;i<L;i++) I[i] = i+L;

	int curr_state = 0;
	int curr_symbol = 0;
	int act = 0;

	for(int x = 0; x<L; x++){
		if((act++)==occ[curr_symbol]) act = 0;
		if(act==0) curr_symbol++;

		table[x].symbol = curr_symbol;
		table[x].input_state = I_s[curr_symbol]++;
		table[x].output_state = I[curr_state];
		curr_state = (curr_state + (int)(0.625*(float)L) + 3)%L;
	}
}

static int tansEncodeSymbol(uint8_t symbol, bitstream_state_t* stream){
	int I_s = tans_state;
	int I_s_max = occ[symbol]*2-1;
	int nBits = log2(b);
	int bits = 0;

	while(I_s > I_s_max){
		int output_val = I_s % b;
		bitstream_append_bits(stream,output_val,nBits);
		bits += nBits;
		I_s = div(I_s,b).quot;
	}

	tans_state = table[offset[symbol]+I_s-occ[symbol]].output_state;
	return bits;
}

int tansEncode(int* n, int size, unsigned char* output, size_t output_buffer_size, bool new_table, int method){
	int parameter;
	if(new_table){
		switch(method)
		{
			case 0:
				parameter = setOccurrences(n,size,L,occ);
				break;
			case 1:
				parameter = setOccurrencesNormal(n,size,L,occ);
				break;
			default:
				break;
		}
		buildTable();
	}

	bitstream_state_t state;
	bitstream_state_t* state_p = &state;
	bitstream_init(state_p, output, output_buffer_size);

	int bits = 0;
	for(int i =0;i<size;i++){
	    bits += tansEncodeSymbol((uint8_t)n[i],state_p);
	}

	bitstream_append_bits(state_p, tans_state, 16);
	bits += 16;
	bitstream_write_close(state_p);
	return bits;
}

static int getIdxFromState(int state){
	int idx = 0;
	while(1){
		if(table[idx].output_state == state){
			return idx;
		}
		idx++;
		if(idx>L) return 999;
	}
}

void tansDecode(unsigned char* input, size_t output_buffer_size, int bits, int size, int* n){
	bitstream_state_t state;
	bitstream_state_t* state_p = &state;
	int bytes = (bits-bits%8)/8;
	bitstream_init_from_behind(state_p, &input[bytes], output_buffer_size, bits%8);
	unsigned long long temp;
	bitstream_read_bits_from_behind(state_p,&temp,16);
	tans_dec_state = (int)temp;
	bits -= 16;

	int min_new_state = L;
	int n_idx = size-1;
	int table_idx;
	int nBits = log2(b);

	while(bits>0){
		table_idx = getIdxFromState(tans_dec_state);
		n[n_idx--]=table[table_idx].symbol;

		tans_dec_state = table[table_idx].input_state;
		while(tans_dec_state < min_new_state){
			 tans_dec_state *= b;
			 unsigned long long new_bits = 0;
			 bitstream_read_bits_from_behind(state_p,&new_bits,nBits);
			 bits -= nBits;
		     tans_dec_state += new_bits;
		}
	}
}

