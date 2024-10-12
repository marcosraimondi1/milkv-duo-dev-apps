#ifndef ENCODING_H
#define ENCODING_H

#include "pb_encode.h"
#include "pb_decode.h"
#include "message.pb.h" // Include the generated header file

#define ERROR   -1
#define SUCCESS 0
#define NUM_CATEOGRIES 3

struct prediction_t {
	char label[32];
	float value;
};

struct timing_t {
	int dsp;
	int classification;
};

struct result_t {
	struct prediction_t predictions[NUM_CATEOGRIES];
	struct timing_t timing;
};

int encode_msg(struct result_t result, uint8_t *buffer, size_t buffer_size);
int decode_msg(struct result_t *result, uint8_t *buffer, size_t message_length);

#endif // !ENCODING_H

