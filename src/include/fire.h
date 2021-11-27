//by Norbert Niderla, 2021

#ifndef SYS_CODER_INCLUDE_FIRE_CODER_H
#define	SYS_CODER_INCLUDE_FIRE_CODER_H

#define FIRE_LEARN_SHIFT               (-1)
#define FIRE_BIT_WIDTH                 (16)

typedef struct fire_coder{
	int learn_shift;
    int bit_width;
    int acc;
    int delta;   
    int first;
}fire_coder_t;

void fire_encode(int* data, unsigned size, fire_coder_t* state);
void fire_decode(int* data, unsigned size, fire_coder_t* state);
void fire_init(int learn_shift, int bit_width, fire_coder_t* state);
void fire_reset(fire_coder_t* state);

#endif
