#include <stdlib.h>
#include <math.h>
#include "fire.h"

#define DIM 1

typedef struct fire_state_s{
	int learn_shift;
    int bit_width;
    int acc[DIM];
    int delta[DIM];
    int first[DIM];
}fire_state;

static fire_state enc_state = {1, 8, {0}, {0}, {0}};
static fire_state dec_state = {1, 8, {0}, {0}, {0}};

void resetFire(void){
	for(int i =0; i < DIM; i++){
		enc_state.acc[i] = 0;
		enc_state.delta[i] = 0;
		enc_state.first[i] = 0;
		dec_state.acc[i] = 0;
		dec_state.delta[i] = 0;
		dec_state.first[i] = 0;
	}
}

static int predict(int x, fire_state* state, int idx){
    int alpha = state->acc[idx] >> state->learn_shift;
    int delta = (alpha*state->delta[idx]) >> state->bit_width;
    return x+delta;
}

static void train(int x, int x_n, int err, fire_state* state, int idx){
    state->acc[idx] =- -copysign(1.0, err)*state->delta[idx];
    state->delta[idx] = x_n - x;
}

void fireEncode(const int* data, int size, int* output){
	for(int k=0;k<DIM;k++){
		int temp = predict(enc_state.first[k], &enc_state, k);
		train(enc_state.first[k], data[k],data[k]-temp,&enc_state, k);
		for(int i = k; i<DIM*(size-1); i+=DIM){
			output[i] = temp - data[i];
			temp = predict(data[i], &enc_state,k);
			train(data[i],data[i+DIM],data[i+DIM]-temp, &enc_state,k);
		}
		output[DIM*(size-1)+k] = temp - data[DIM*(size-1)+k];
		enc_state.first[k] = data[DIM*(size-1)+k];
	}
}

void fireDecode(int* data, int size, int* output){
	for(int k=0;k<DIM;k++){
		int temp = predict(dec_state.first[k], &dec_state, k);
		train(dec_state.first[k], temp-data[k],-data[k],&dec_state, k);

		for(int i = k; i<DIM*(size-1); i+=DIM){
			output[i] = temp - data[i];
			temp = predict(output[i], &dec_state, k);
			train(output[i],temp-data[i+DIM],-data[i+DIM], &dec_state, k);
		}
		output[DIM*(size-1)+k] = temp - data[DIM*(size-1)+k];
		dec_state.first[k] = output[DIM*(size-1)+k];
	}
}
