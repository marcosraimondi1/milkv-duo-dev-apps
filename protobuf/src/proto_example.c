#include <stdio.h>
#include "encoding.h"

int main()
{
	/* This is the buffer where we will store our message. */
	uint8_t buffer[128];
	char board[3][3] = {{'X', 'O', 'X'}, {'O', 'X', 'O'}, {'X', 'O', 'X'}};
	char board_decoded[3][3] = {0};
	int status;

	status = encode_board(board, buffer, sizeof(buffer));
	if (status < 0) {
		printf("Encoding failed\n");
		return 1;
	}

	status = decode_board(board_decoded, buffer, sizeof(buffer));
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

	return 0;
}
