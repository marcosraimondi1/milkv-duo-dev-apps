#include "comms.h"
#include "minimax.h"
#include "encoding.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int user_turn;
int machine_vs_machine = 0;
char BOARD[3][3];
char play_marker;
uint8_t buffer[128];

void setup(void);
void loop(void);
int send_board(char board[3][3]);
int recv_board(char board[3][3]);

int main(int argc, char *argv[])
{
	printf("Tic Tac Toe\n");
	int status = rpmsg_init_netlink();

	if (status < 0) {
		printf("rpmsg_init_netlink failed\n");
		return EXIT_FAILURE;
	}

	setup();
	loop();

	rpmsg_exit_netlink();
	return EXIT_SUCCESS;
}

void setup()
{
	initial_state(BOARD);
	fprintf(stderr, "Do you want to play first? (1 for yes, 0 for no, -1 MvsM): ");
	scanf("%d", &user_turn);
	if (user_turn == -1) {
		machine_vs_machine = 1;
	}
	user_turn = user_turn ? 1 : 0;
	play_marker = user_turn ? X : O;
}

void loop()
{
	struct action best_move;
	struct action user_move;

	while (!terminal(BOARD)) {
		draw(BOARD);
		if (machine_vs_machine) {
			sleep(1);
		} else {
			printf("You are %c\n", play_marker);
		}

		if (user_turn) {
			if (machine_vs_machine) {
				minimax(BOARD, &best_move);
				BOARD[best_move.row][best_move.col] = player(BOARD);
			} else {
				fprintf(stderr, "Enter your move (row column): ");
				while (1) {
					scanf("%d %d", &user_move.row, &user_move.col);
					if (user_move.row < 0 || user_move.row > 2 ||
					    user_move.col < 0 || user_move.col > 2) {
						fprintf(stderr, "Invalid move, try again: ");
						continue;
					}
					if (BOARD[user_move.row][user_move.col] == EMPTY) {
						break;
					}
					fprintf(stderr, "Invalid move, try again: ");
				}
				BOARD[user_move.row][user_move.col] = player(BOARD);
			}
			user_turn = 0;
		} else {
			// send the board to the minimax algorithm in remote processor
			if (send_board(BOARD) < 0) {
				printf("send_board failed\n");
				return;
			}

			// receive the best move from the remote processor
			if (recv_board(BOARD) < 0) {
				printf("recv_board failed\n");
				return;
			}

			user_turn = 1;
		}
	}

	draw(BOARD);
	if (!machine_vs_machine) {
		printf("You are %c\n", play_marker);
	}

	if (winner(BOARD) == X) {
		printf("X wins!\n");
	} else if (winner(BOARD) == O) {
		printf("O wins!\n");
	} else {
		printf("It's a draw!\n");
	}
}

int encoded_size = 0;
int send_board(char board[3][3])
{
	encoded_size = encode_board(BOARD, buffer, sizeof(buffer));

	if (encoded_size < 0) {
		printf("encode_board failed\n");
		return -1;
	}

	return send_msg(buffer, encoded_size);
}

int recv_board(char board[3][3])
{
	int ret = recv_msg(buffer, encoded_size);

	if (ret < 0) {
		printf("recv_msg failed\n");
		return -1;
	}

	ret = decode_board(BOARD, buffer);

	return ret;
}
