#include <stdio.h>
#include "encoding.h"

int main(int argc, char *argv[])
{
	if (argc != 2) {
		printf("Usage: %s <file>\n", argv[0]);
		return 1;
	}

	uint8_t buffer[128];

	// read encoded message from file
	FILE *file = fopen(argv[1], "rb");
	if (!file) {
		return 0;
	}
	size_t message_length = fread(buffer, 1, sizeof(buffer), file);

	struct result_t result;
	int status = decode_msg(&result, buffer, message_length);
	if (status < 0) {
		printf("Decoding failed\n");
		return 1;
	}

	printf("DSP: %d ms, Classification: %d ms\n", result.timing.dsp,
	       result.timing.classification);
	for (int i = 0; i < 3; i++) {
		printf("%s: %.2f %%\n", result.predictions[i].label,
		       100.0 * result.predictions[i].value);
	}

	return 0;
}
