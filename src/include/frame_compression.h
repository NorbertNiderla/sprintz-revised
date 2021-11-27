/*
 * frame_compression.h
 *
 *  Created on: Sep 22, 2021
 *      Author: Norbert Niderla
 *      		Warsaw University of Technology
 */

#ifndef COMPRESSION_IOT_INCLUDE_FRAME_COMPRESSION_H_
#define COMPRESSION_IOT_INCLUDE_FRAME_COMPRESSION_H_


#define SINGLE_FRAME_ENCODING	(0)
#define ACK_TRANSMISSION_ENCODING	(1)
/*
typedef struct frame_compression_config{
	unsigned char* buffer;
	const int* data;
	unsigned size_low_limit;
	unsigned size_high_limit;
	int output_bytes;
	unsigned reset_option;
	unsigned char* additional_output_buffer;
	unsigned additional_output_buffer_size;
} frame_compression_config_t;
*/
unsigned get_fire_rice_frame(unsigned char* buffer,
								const int* data,
								unsigned size_low_limit,
								unsigned size_high_limit,
								int* output_bytes
#if ACK_TRANSMISSION_ENCODING
								,unsigned reset_state
#endif
								);

unsigned get_sprintz_frame(unsigned char* buffer,
								const int* data,
								unsigned size_low_limit,
								unsigned size_high_limit,
								int* output_bytes
#if ACK_TRANSMISSION_ENCODING
								,unsigned reset_state
#endif
								);

unsigned get_sprintz_huffman_frame(unsigned char* buffer,
								const int* data,
								unsigned size_low_limit,
								unsigned size_high_limit,
								int* output_bytes,
#if !ACK_TRANSMISSION_ENCODING
                                unsigned reset_huffman_tree,
#endif
                                unsigned char* output_buffer,
                                unsigned output_buffer_size
#if ACK_TRANSMISSION_ENCODING
								,unsigned reset_state
#endif
								);

unsigned get_sprintz_tans_frame(unsigned char* buffer,
								const int* data,
								unsigned size_low_limit,
								unsigned size_high_limit,
								int* output_bytes,
                                unsigned char* output_buffer,
                                unsigned output_buffer_size
#if ACK_TRANSMISSION_ENCODING
								,unsigned reset_state
#endif
								);

#endif /* COMPRESSION_IOT_INCLUDE_FRAME_COMPRESSION_H_ */
