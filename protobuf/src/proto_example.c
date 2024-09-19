#include <stdio.h>
#include "encoding.h"

int main()
{
	/* This is the buffer where we will store our message. */
	uint8_t buffer[128];
	char board[3][3] = {{'X', 'X', 'X'}, {'O', ' ', 'O'}, {' ', 'O', ' '}};
	char board_decoded[3][3] = {0};
	int message_length;

	message_length = encode_board(board, buffer, sizeof(buffer));
	if (message_length < 0) {
		printf("Encoding failed\n");
		return 1;
	}

	printf("Encoded %d bytes\n", message_length);

	// write encoded message to file
	FILE *f = fopen("c_encoded", "wb");
	fwrite(buffer, 1, message_length, f);
	fclose(f);

	int status = decode_board(board_decoded, buffer, (size_t)message_length);
	if (status < 0) {
		printf("Decoding failed\n");
		return 1;
	}

	for (int i = 0; i < 3; i++) {
		for (int j = 0; j < 3; j++) {
			printf("%c=%c ", board[i][j], board_decoded[i][j]);
		}
		printf("\n");
	}

	// read python encoded message
	FILE *f2 = fopen("python_encoded", "rb");
	if (!f2) {
		return 0;
	}
	uint8_t buffer2[128];
	size_t message_length2 = fread(buffer2, 1, sizeof(buffer2), f2);

	char board_decoded2[3][3] = {0};
	status = decode_board(board_decoded2, buffer2, message_length2);
	if (status < 0) {
		printf("Decoding python failed\n");
		return 1;
	}

	printf("Decoded python message\n");
	for (int i = 0; i < 3; i++) {
		for (int j = 0; j < 3; j++) {
			printf("%c ", board_decoded2[i][j]);
		}
		printf("\n");
	}

	return 0;
}
