//by Norbert Niderla, 2020

#include <stdlib.h>
#include <math.h>
#include "rice.h"
#include "bitstream.h"

#define R_PARAMETER_PRECISE (0)

#define ENABLE_DEBUG	(0)
#include "debug.h"

static void zigzag_encode(int* data, int size){
	for(int i = 0; i < size; i++)
		data[i] = (data[i] >> 31) ^ (data[i] << 1);
}

static void zigzag_decode(int* data, int size){
	for(int i = 0; i < size; i++)
		data[i] = (data[i] >> 1) ^ -(data[i] & 1);
}

#if R_PARAMETER_PRECISE
static int std_min_bit_length(int x){
  if(x == 0) return 0;

  return(int)(floor(log2(abs(x))) + 1);
}

static int findAbsoluteSmallestValue(int* arr, int size){
	int i = 0;
	int val = abs(arr[0]);
	while(i<size){
		if(abs(arr[i])<val) val = abs(arr[i]);
		i++;
	}
	return val;
}

static int findAbsoluteBiggestValue(int* arr, int size){
	int i = 1;
	int val = arr[0];
	while(i<size){
		if(abs(arr[i])>val) val = abs(arr[i]);
		i++;
	}
	return val;
}

static int rice_parameter_estimate(int* n, int size, int offset){
	int R_min = -1;
	int R_max = -1;

	if(R_min < 0){
		R_min = std_min_bit_length(findAbsoluteSmallestValue(n, size)) - 1;
		if(R_min<0) R_min = 0;
	}

	if(R_max < 0) R_max = std_min_bit_length(findAbsoluteBiggestValue(n, size));

	int s, r_val;
	r_val = 999;
	int b_val = 999;

	//printf("%d %d\n", R_min,R_max);//DEBUG
	for(int r = R_min; r <= R_max; r++){
		s = 0;
		for(int i = offset; i<DIM*size; i+=DIM){
			s += (abs(n[i]))>>r;
		}

		if((s + size*(r + 2))<b_val){
			b_val = (s + size*(r + 2));
			r_val = r;
		}
	}
	return r_val;
}

#else
/*
static int rice_parameter_estimate(int* n, int size){
	int sum = 0;
	for(int i = 0; i < size; i++) sum += n[i];
	if(sum==0) return 0;
	float mu = (float)sum/(float)size;
	float r = ceilf(log2f(logf(2)*mu));
	if(r<0) return 0;
 	return r;
}
*/

static int rice_parameter_estimate(int* n, int size){
	int sum = 0;
	for(int i = 0; i < size; i++) sum += n[i];
	if(sum==0) return 0;
	int mu = (int)ceilf((float)sum/(float)size);
	int r = 32 - __builtin_clz((mu*6)>>3);
 	return r;
}
#endif

/*
int rice_encode(int* n, int size, int* output){

	zigzag_encode(n,size);
	uint32_t r = rice_parameter_estimate(n, size);

	uint32_t sym,s,value = 0;

	for(int i = 0; i<size; i++){
		sym = n[i];
		s = sym>>r;
		if(s!=0) value |= (~(0xFFFFFFFF<<s))<<(r+1);
		value |= sym&(~(0xFFFFFFFF<<r));
		output[i] = (int)value;
		value = 0;
	}
	return (int)r;
}

void riceDecode(int* input, int size, int* n, unsigned long long r){
	int m,s;
	int bit = 0;
	for(int i = 0; i<size*DIM; i++){
		m = input[i] & (~(0xFFFFFFFF<<r));
		s = 0;
		while(bit!=0){
			bit = input[i] & (1<<(r+1+s));
		}
		n[i] = (s<<r) + m;
		zigzagDecode(n,size*DIM);
	}
}
*/

void rice_encode(int* n, int size, bitstream_state_t* stream){

	div_t loops = div(size, RICE_BATCH);
	zigzag_encode(n,size);
	
	for(int x = 0; x < loops.quot; x++){
		int r = rice_parameter_estimate(&n[x*RICE_BATCH], RICE_BATCH);
		if(r > R_MAX) DEBUG("rice.c: rice_encode: r is too big!\n");
		DEBUG("r_parameter: %d\n", r);

		bitstream_append_bits(stream,r,R_ENCODE_BITS);

		for(int i = 0; i<RICE_BATCH; i++){
			if(r == 0){
				int d = n[x*RICE_BATCH + i];
				int s = d;
				if(s > 32) DEBUG("s is too big\n");
				bitstream_append_bits(stream, (~(0xFFFFFFFF<<s)),s);
				bitstream_append(stream, 0, 1);
			} else {
				int d = n[x*RICE_BATCH + i];
				int s = d >> r;
				bitstream_append_bits(stream, (~(0xFFFFFFFF<<s)),s);
				bitstream_append(stream, 0, 1);
				bitstream_append_bits(stream, d, r);
			}
		}
	}

	if(loops.rem != 0){
		int r = rice_parameter_estimate(&n[loops.quot*RICE_BATCH], loops.rem);
		if(r > R_MAX) DEBUG("rice.c: rice_encode: r is too big!\n");
		bitstream_append_bits(stream,r,R_ENCODE_BITS);

		for(int i = 0; i<loops.rem; i++){
			int d = n[loops.quot*RICE_BATCH + i];
			int s = d >> r;
			bitstream_append_bits(stream, (~(0xFFFFFFFF<<s)),s);
			bitstream_append(stream, 0, 1);
			bitstream_append_bits(stream, d, r);
		}
	}

	bitstream_write_close(stream);
}

unsigned rice_decode(unsigned char* input, int bytes, int* n){
	bitstream_state_t state;
	bitstream_init(&state, input, bytes);
	unsigned long long m, bit = 0, r = 0;
	int i = 0;
	while(state.stream_free_len > 0){
		bitstream_read_bits(&state, &r, R_ENCODE_BITS);
		for(int x = 0; x < RICE_BATCH; x++){
			int s = 0;
			bitstream_read_bits(&state, &bit, 1);
			while((bit!=0) & (state.stream_free_len > 0)){
				s++;
				bitstream_read_bits(&state,&bit,1);
			}
			bitstream_read_bits(&state, &m, r);
			n[i++] = (s << r) + m;
			if(state.stream_free_len == 0) break;
		}
	}
	zigzag_decode(n, i);
	return i;
}
