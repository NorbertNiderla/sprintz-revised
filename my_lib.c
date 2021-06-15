#include <stdio.h>
#include <stdlib.h>
#include "my_lib.h"

void clearCharArray(unsigned char* arr, int size){
	for(int idx=0;idx<size;idx++) arr[idx]=0;
}

void clearIntArray(int* arr, int size){
	for(int idx=0;idx<size;idx++) arr[idx]=0;
}
