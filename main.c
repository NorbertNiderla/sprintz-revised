#include <stdio.h>
#include <math.h>
#include <stdbool.h>

#include "include/distribution.h"
#include "include/fire.h"
#include "include/huffman.h"
#include "include/my_lib.h"
#include "include/rice.h"
#include "include/sprintz.h"
#include "include/tans.h"
#include "xtimer.h"

#define BUFFER_SIZE 	(128)
#define ENABLE_DEBUG 	(0)
#define RETURN_BITS		(1)

static fire_coder_t fire_state;

int sprintz_tans_testing(const int* data, int stack_n, int table_build_coeff){
		xtimer_ticks32_t one;
		xtimer_ticks32_t two;
		xtimer_ticks32_t sum_time;

		int sum_bits = 0;
		sum_time.ticks32 = 0;
		int idx = 0;
		unsigned char char_buffer[BUFFER_SIZE] = {0};
		fire_coder_init(FIRE_LEARN_SHIFT, FIRE_BIT_WIDTH, &fire_state);
		bool new_table;
		while(idx<DATA_SIZE){

			one = xtimer_now();

			if(idx%(stack_n*table_build_coeff) ==0){
				new_table = true;
			}
			else new_table = false;

			int bits = sprintzEncode_tans(&data[idx],stack_n,char_buffer,BUFFER_SIZE, new_table, &fire_state);
			two = xtimer_now();

			clearCharArray(char_buffer,BUFFER_SIZE);
			idx += stack_n;
			sum_bits += bits;
			sum_time.ticks32 += xtimer_diff(two,one).ticks32;
		}

#if RETURN_BITS
	return(sum_bits);
#else
	return(sum_time.ticks32);
#endif
}


int sprintz_testing(const int* data, int stack_n, int table_build_coeff){
		xtimer_ticks32_t one;
		xtimer_ticks32_t two;
		xtimer_ticks32_t sum_time;

		int sum_bits = 0;
		sum_time.ticks32 = 0;
		int idx = 0;
		unsigned char char_buffer[BUFFER_SIZE] = {0};
		fire_coder_init(FIRE_LEARN_SHIFT, FIRE_BIT_WIDTH, &fire_state);
		bool new_table;
		while(idx<DATA_SIZE){

			one = xtimer_now();

			if(idx%(stack_n*table_build_coeff) ==0){
				new_table = true;
			}
			else new_table = false;

			int bits = sprintzEncode(&data[idx],stack_n,char_buffer,BUFFER_SIZE, new_table, &fire_state);
			two = xtimer_now();

			clearCharArray(char_buffer,BUFFER_SIZE);
			idx += stack_n;
			sum_bits += bits;
			sum_time.ticks32 += xtimer_diff(two,one).ticks32;
		}

#if RETURN_BITS
	return(sum_bits);
#else
	return(sum_time.ticks32);
#endif
}

int fire_rice_testing(const int* data, int stack_n){
	xtimer_ticks32_t one;
	xtimer_ticks32_t two;
	xtimer_ticks32_t sum_time;

 	int int_buffer[BUFFER_SIZE] = {0};
	unsigned char char_buffer[BUFFER_SIZE] = {0};

	int sum_bits = 0;
	sum_time.ticks32 = 0;
	int idx = 0;
	fire_coder_init(FIRE_LEARN_SHIFT, FIRE_BIT_WIDTH, &fire_state);
		while(idx<DATA_SIZE){
			one = xtimer_now();

			fireEncode(&data[idx],stack_n,int_buffer, &fire_state);
			int bits = riceEncodeStream(int_buffer, stack_n, char_buffer, BUFFER_SIZE);

			two = xtimer_now();

			sum_bits += bits;
			sum_time.ticks32 += xtimer_diff(two,one).ticks32;

			clearCharArray(char_buffer,BUFFER_SIZE);
			clearIntArray(int_buffer,BUFFER_SIZE);
			idx += stack_n;
		}
#if RETURN_BITS
	return(sum_bits);
#else
	return(sum_time.ticks32);
#endif
}

int main(void){
	/*
	*
	* testing
	*
	*
	*/
	return 0;
}
