#include <stdio.h>
#include <math.h>
#include <stdbool.h>
#include "fire.h"
#include "rice.h"
#include "tans.h"
#include "sprintz.h"

#define BUFFER_SIZE 128 //buffer size should be twice (or more) of length of input stream
#define DIM 1 //dimentiality of data, remember to change it in all files

int FIRE_Rice_tANS(int* data, int size, int* int_buffer, unsigned char* char_buffer, bool new_table, int distribution_method){
	fireEncode(data,size,int_buffer);
	riceEncode(int_buffer, size, int_buffer);
	int tans_bits = tansEncode(int_buffer,size,char_buffer,BUFFER_SIZE, new_table, distribution_method);
	return tans_bits;
}

int Sprintz(int* data, int size, unsigned char* char_buffer, bool new_table){
	int bytes = sprintzEncode(data,size,char_buffer,BUFFER_SIZE, new_table);
	return bytes*8;
}	

int FIRE_Rice(int* data, int size, int* int_buffer, unsigned char* char_buffer){
	fireEncode(&data[idx],stack_n[k_tab],int_buffer);
	int bits = riceEncode(int_buffer, stack_n[k_tab], char_buffer,BUFFER_SIZE);
	return bits;
}

int FIRE_tANS(int* data, int size, int* int_buffer, unsigned char* char_buffer, bool new_table, int distribution_method){
	fireEncode(&data[idx],stack_n[k_tab],int_buffer);
	zigzag(int_buffer, stack_n[k_tab]);
	int tans_bits = tansEncode(int_buffer,stack_n[k_tab],char_buffer,BUFFER_SIZE, new_table, distribution_method);
}

int main(void){
 	int int_buffer[BUFFER_SIZE] = {0};
	unsigned char char_buffer[BUFFER_SIZE] = {0};
}

