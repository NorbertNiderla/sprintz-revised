/**
 * by Norbert Niderla, 2021
 * 
 * Warsaw University of Technology
 */

#ifndef COMPRESSION_IOT_RICE_H
#define COMPRESSION_IOT_RICE_H

#include "bitstream.h"

#define R_ENCODE_BITS	(5)
#define R_MAX			(31)
#define RICE_BATCH		(10)

void rice_encode(int* n, int size, bitstream_state_t* stream);
unsigned rice_decode(unsigned char* input, int bytes, int* n);

#endif
