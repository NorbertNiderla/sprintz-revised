//by Norbert Niderla, 2021

#include <stdint.h>
#include "fire.h"

static uint32_t predict(uint32_t x, fire_coder_t* state){
    int32_t alpha;
	if(state->learn_shift>0){
		alpha = state->acc >> state->learn_shift;
	}
	else if(state->learn_shift<0){
		alpha = state->acc << -state->learn_shift;
	}
	else{
		alpha = state->acc;
	}

	if(state->delta>=0){
		return x + ((alpha*state->delta) >> state->bit_width);
	}
	else if(state->delta<0){
		return x - ((-alpha*state->delta) >> state->bit_width);
	}

	return 0;
}

static void train(uint32_t x, uint32_t x_n, int32_t err, fire_coder_t* state){
    if(err >= 0){
		state->acc += state->delta;
	} else {
		state->acc -= state->delta;
	}
	
    if(state->acc < 0 ) state->acc = 0;
    state->delta = x_n - x;
}

void fire_init(int learn_shift, int bit_width, fire_coder_t* state){
	state->learn_shift = learn_shift;
	state->bit_width = bit_width;
	state->delta = 0;
	state->acc = 0;
	state->first = 0;
}

void fire_reset(fire_coder_t* state){
	state->delta = 0;
	state->acc = 0;
	state->first = 0;
}

void fire_encode(int* data, unsigned size, fire_coder_t* state){

	int32_t temp = predict(state->first, state);
	train(state->first, data[0], data[0]-temp, state);
	state->first = data[size-1];
	int32_t output_value;

	for(uint8_t i = 0; i<(size-1); i++){
		output_value = temp - data[i];
		temp = predict(data[i], state);
		train(data[i], data[i+1], data[i+1]-temp, state);
		data[i] = output_value;
	}

	output_value = temp - data[size-1];
	data[size-1] = output_value;
}

void fire_decode(int* data, unsigned size, fire_coder_t* state){

	uint32_t temp = predict(state->first, state);
	int32_t output_value;
	int32_t n = data[0];
	train(state->first, temp-n,-n, state);


	for(uint8_t i = 0; i<(size-1); i++){
		output_value = temp - data[i];
		temp = predict(output_value, state);
		train(output_value,temp-data[i+1],-data[i+1], state);
		data[i] = output_value;
	}

	output_value = temp - data[size-1];
	data[size-1] = output_value;
	state->first = output_value;
}
