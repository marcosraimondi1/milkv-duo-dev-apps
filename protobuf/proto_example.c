#include <stdio.h>
#include <string.h>
#include "pb_encode.h"
#include "pb_decode.h"
#include "message.pb.h" // Include the generated header file

int main()
{
	/* This is the buffer where we will store our message. */
	uint8_t buffer[128];
	size_t message_length;
	bool status;

	/* Encode our message */
	{
		/* Initialize the message structure to zero */
		SimpleMessage message = SimpleMessage_init_zero;

		/* Set the values of the array */
		int32_t board_data[] = {1, 2, 3, 4, 5};
		size_t board_size = sizeof(board_data) / sizeof(board_data[0]);

		/* Copy the data to the message */
		memcpy(message.board, board_data, board_size * sizeof(int32_t));
		message.board_count = board_size; // Set the actual number of elements

		/* Create a stream that will write to our buffer */
		pb_ostream_t stream = pb_ostream_from_buffer(buffer, sizeof(buffer));

		/* Encode the message */
		status = pb_encode(&stream, SimpleMessage_fields, &message);
		message_length = stream.bytes_written;

		/* Check for encoding errors */
		if (!status) {
			printf("Encoding failed: %s\n", PB_GET_ERROR(&stream));
			return 1;
		}
		printf("Message encoded successfully!\n");
	}

	printf("Encoded message length: %zu\n", message_length);

	/* Decode the message */
	{
		/* Initialize the message structure to zero */
		SimpleMessage message = SimpleMessage_init_zero;

		/* Create a stream that reads from the buffer */
		pb_istream_t stream = pb_istream_from_buffer(buffer, message_length);

		/* Decode the message */
		status = pb_decode(&stream, SimpleMessage_fields, &message);

		/* Check for decoding errors */
		if (!status) {
			printf("Decoding failed: %s\n", PB_GET_ERROR(&stream));
			return 1;
		}

		/* Print the decoded data */
		printf("Decoded message contains %hu elements:\n", message.board_count);
		for (size_t i = 0; i < message.board_count; ++i) {
			printf("Element %zu: %d\n", i, message.board[i]);
		}
	}

	return 0;
}
