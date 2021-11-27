/** 
 * by Norbert Niderla, 2021
 * 
 * tANS by Jaroslaw Duda
 * 
 */


#ifndef COMPRESSION_IOT_INCLUDE_TANS_H_
#define COMPRESSION_IOT_INCLUDE_TANS_H_

#define TANS_ADAPTIVE   (1)

#define TANS_ENCODER_ON	(1)
#define TANS_DECODER_ON	(0)

#if TANS_ENCODER_ON
#if TANS_ADAPTIVE
unsigned tans_encode(unsigned char* data, int size, unsigned char *output, size_t output_buffer_size, unsigned reset_state);
#else
unsigned tans_encode(unsigned char* data, int size, unsigned char *output, size_t output_buffer_size);
#endif
#endif
#if TANS_DECODER_ON
void tans_decode(unsigned char *input, unsigned bytes, unsigned char* data);
#endif

#endif
