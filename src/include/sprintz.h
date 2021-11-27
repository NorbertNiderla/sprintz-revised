/**
 * by Norbert Niderla, 2021
 * 
 * Sprintz by Blalock 
 */


#ifndef COMPRESSION_IOT_INCLUDE_SPRINTZ_H_
#define COMPRESSION_IOT_INCLUDE_SPRINTZ_H_
#include "bitstream.h"

#define BITPACKING_BATCH	(8)

void sprintz_encode(int* data,
                    int size, 
                    bitstream_state_t* stream,
                    unsigned reset_state);

unsigned sprintz_decode(bitstream_state_t* stream,
                        unsigned  size,
                        int* data,
                        unsigned reset_state);

#endif