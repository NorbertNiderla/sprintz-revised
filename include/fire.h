//by Norbert Niderla, 2021

#ifndef SYS_CODER_INCLUDE_FIRE_CODER_H
#define	SYS_CODER_INCLUDE_FIRE_CODER_H

#include <stdint.h>

#define FIRE_LEARN_SHIFT               (-1)
#define FIRE_BIT_WIDTH                 (16)

typedef struct fire_coder{
	int learn_shift;
    int bit_width;
    int acc;
    int delta;
    int first;
}fire_coder_t;

void fireEncode(const int* data, uint8_t size, int* output, fire_coder_t* state);
void fireDecode(const int* data, uint8_t size, int* output, fire_coder_t* state);


void fire_coder_init(int learn_shift, int bit_width, fire_coder_t* state);

#endif
