#include <sys/select.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <stdint.h>
#include <stddef.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <time.h>
#include <stdbool.h>
#include <semaphore.h>
#include <linux/rpmsg.h>
#include <sys/socket.h>
#include <linux/netlink.h>
#include <errno.h>
#include <limits.h>

#define NETLINK_ID 18

#define NUM_ITERATIONS_DEFAULT 10

/* message size options */
#define RPMSG_BUFFER_SIZE 512 // as defined in the kernel (includes header)
#define MSG_SIZE_DEFAULT  (RPMSG_BUFFER_SIZE - 16)

int fd;
int user_turn;
int machine_vs_machine = 0;
char BOARD[3][3];
char play_marker;
struct sockaddr_nl src_addr, dest_addr;
struct nlmsghdr *nlh_send, *nlh_recv;

int rpmsg_init_netlink(void);
int rpmsg_exit_netlink(int fd);
int send_msg_netlink(int fd, struct nlmsghdr *nlh, struct sockaddr_nl dest_addr);
int recv_msg_netlink(int fd, struct nlmsghdr *nlh, int len);

int run_app(void);

#define X     'X'
#define O     'O'
#define EMPTY ' '

struct action {
	int row;
	int col;
};

/**
 * @brief Initialize the board to the initial state
 */
void initial_state(char board[3][3])
{
	for (int i = 0; i < 3; i++) {
		for (int j = 0; j < 3; j++) {
			board[i][j] = EMPTY;
		}
	}
}

/**
 * @brief Determine the current player
 * @param board The current board
 * @return char The current player
 */
char player(char board[3][3])
{
	int x_count = 0, o_count = 0;
	for (int i = 0; i < 3; i++) {
		for (int j = 0; j < 3; j++) {
			if (board[i][j] == X) {
				x_count++;
			} else if (board[i][j] == O) {
				o_count++;
			}
		}
	}
	return (x_count > o_count) ? O : X;
}

/**
 * @brief Determine the available actions
 * @param board The current board
 * @param action_list The list of available actions
 * @return int The number of available actions
 */
int actions(char board[3][3], struct action action_list[9])
{
	int count = 0;
	for (int i = 0; i < 3; i++) {
		for (int j = 0; j < 3; j++) {
			if (board[i][j] == EMPTY) {
				action_list[count].row = i;
				action_list[count].col = j;
				count++;
			}
		}
	}
	return count;
}

/**
 * @brief Apply a move and return a new board
 * @param new_board The new board
 * @param board The current board
 * @param action The action to apply
 */
void result(char new_board[3][3], char board[3][3], struct action action)
{
	for (int k = 0; k < 3; k++) {
		for (int l = 0; l < 3; l++) {
			new_board[k][l] = board[k][l];
		}
	}
	new_board[action.row][action.col] = player(board);
}

/**
 * @brief Check for a winner
 * @param board The current board
 * @return char The winner
 */
char winner(char board[3][3])
{
	for (int i = 0; i < 3; i++) {
		if (board[i][0] == board[i][1] && board[i][0] == board[i][2] &&
		    board[i][0] != EMPTY) {
			return board[i][0];
		}
		if (board[0][i] == board[1][i] && board[0][i] == board[2][i] &&
		    board[0][i] != EMPTY) {
			return board[0][i];
		}
	}
	if (board[0][0] == board[1][1] && board[0][0] == board[2][2] && board[0][0] != EMPTY) {
		return board[0][0];
	}
	if (board[0][2] == board[1][1] && board[0][2] == board[2][0] && board[0][2] != EMPTY) {
		return board[0][2];
	}
	return EMPTY;
}

/**
 * @brief Check if the game is over
 * @param board The current board
 * @return int 1 if the game is over, 0 otherwise
 */
int terminal(char board[3][3])
{
	if (winner(board) != EMPTY) {
		return 1;
	}
	for (int i = 0; i < 3; i++) {
		for (int j = 0; j < 3; j++) {
			if (board[i][j] == EMPTY) {
				return 0;
			}
		}
	}
	return 1;
}

/**
 * @brief Calculate the utility of the board
 * @param board The current board
 * @return int The utility of the board
 */
int utility(char board[3][3])
{
	char win = winner(board);
	if (win == X) {
		return 1;
	} else if (win == O) {
		return -1;
	} else {
		return 0;
	}
}

// Maximize function
int Max_Value(char board[3][3]);

// Minimize function
int Min_Value(char board[3][3])
{
	if (terminal(board)) {
		return utility(board);
	}

	int v = INT_MAX;
	struct action action_list[9];
	int actions_count = actions(board, action_list);

	for (int i = 0; i < actions_count; i++) {
		char new_board[3][3];
		result(new_board, board, action_list[i]);
		int max_val = Max_Value(new_board);
		if (max_val < v) {
			v = max_val;
		}
	}
	return v;
}

// Maximize function
int Max_Value(char board[3][3])
{
	if (terminal(board)) {
		return utility(board);
	}

	int v = INT_MIN;
	struct action action_list[9];
	int actions_count = actions(board, action_list);

	for (int i = 0; i < actions_count; i++) {
		char new_board[3][3];
		result(new_board, board, action_list[i]);
		int min_val = Min_Value(new_board);
		if (min_val > v) {
			v = min_val;
		}
	}
	return v;
}

/**
 * @brief Minimax algorithm
 * @param board The current board
 * @param best_move The best move
 */
void minimax(char board[3][3], struct action *best_move)
{
	if (terminal(board)) {
		best_move->row = -1;
		best_move->col = -1;
		return;
	}

	int best_val;
	if (player(board) == X) {
		best_val = INT_MIN;
	} else {
		best_val = INT_MAX;
	}

	struct action action_list[9];
	int actions_count = actions(board, action_list);

	for (int i = 0; i < actions_count; i++) {
		char new_board[3][3];
		result(new_board, board, action_list[i]);

		int current_val;
		if (player(board) == X) {
			current_val = Min_Value(new_board);
			if (current_val > best_val) {
				best_val = current_val;
				best_move->row = action_list[i].row;
				best_move->col = action_list[i].col;
			}
		} else {
			current_val = Max_Value(new_board);
			if (current_val < best_val) {
				best_val = current_val;
				best_move->row = action_list[i].row;
				best_move->col = action_list[i].col;
			}
		}
	}
}

/**
 * @brief Draw the board
 * @param board The current board
 */
void draw(char board[3][3])
{
	printf("\033[H\033[J"); // clear screen
	printf("\t0\t1\t2\n\n");
	for (int i = 0; i < 3; i++) {
		printf("%d", i);
		for (int j = 0; j < 3; j++) {
			printf("\t%c", board[i][j]);
		}
		printf("\n\n");
	}
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

void send_board(char board[3][3])
{
	char board_to_send[9] = {0};
	for (int i = 0; i < 3; i++) {
		for (int j = 0; j < 3; j++) {
			board_to_send[i * 3 + j] = BOARD[i][j];
		}
	}
	memcpy(NLMSG_DATA(nlh_send), board_to_send, sizeof(board_to_send));
	nlh_send->nlmsg_len = NLMSG_SPACE(sizeof(board_to_send));
	int ret = send_msg_netlink(fd, nlh_send, dest_addr);
	if (ret < 0) {
		printf("send_msg failed ret = %d\n", ret);
	}
}

void recv_board(char board[3][3])
{
	char board_recv[9] = {0};
	int ret = recv_msg_netlink(fd, nlh_recv, sizeof(board_recv));
	if (ret < 0) {
		printf("recv_msg failed ret = %d\n", ret);
	}
	memcpy(board_recv, NLMSG_DATA(nlh_recv), sizeof(board_recv));
	for (int i = 0; i < 3; i++) {
		for (int j = 0; j < 3; j++) {
			BOARD[i][j] = board_recv[i * 3 + j];
		}
	}
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
			send_board(BOARD);

			// receive the best move from the remote processor
			recv_board(BOARD);

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

int main(int argc, char *argv[])
{
	int status = run_app();
	if (status < 0) {
		printf("APP STATUS: FAILED\n");
	} else {
		printf("APP STATUS: PASSED\n");
	}

	return 0;
}

int run_app()
{
	printf("Tic Tac Toe\n");
	fd = rpmsg_init_netlink();
	printf("fd = %d\n", fd);

	if (fd < 0) {
		printf("rpmsg_init failed\n");
		return -1;
	}

	setup();
	loop();

	rpmsg_exit_netlink(fd);
	return 0;
}

int rpmsg_init_netlink()
{
	int sock_fd;

	// Create netlink socket
	sock_fd = socket(PF_NETLINK, SOCK_RAW, NETLINK_ID);
	if (sock_fd < 0) {
		printf("socket: %s\n", strerror(errno));
		return -1;
	}

	// Bind socket
	memset(&src_addr, 0, sizeof(src_addr));
	src_addr.nl_family = AF_NETLINK;
	src_addr.nl_pid = getpid();
	if (bind(sock_fd, (struct sockaddr *)&src_addr, sizeof(src_addr)) != 0) {
		printf("bind: %s\n", strerror(errno));
		close(sock_fd);
		return -1;
	}

	// Set destination address
	memset(&dest_addr, 0, sizeof(dest_addr));
	dest_addr.nl_family = AF_NETLINK;
	dest_addr.nl_pid = 0; // For Linux Kernel

	// Allocate netlink message header
	nlh_send = (struct nlmsghdr *)malloc(NLMSG_SPACE(RPMSG_BUFFER_SIZE - 16));
	nlh_recv = (struct nlmsghdr *)malloc(NLMSG_SPACE(RPMSG_BUFFER_SIZE - 16));

	return sock_fd;
}

int rpmsg_exit_netlink(int fd)
{
	free(nlh_send);
	free(nlh_recv);
	close(fd);
	return 0;
}

int send_msg_netlink(int fd, struct nlmsghdr *nlh, struct sockaddr_nl dest_addr)
{
	int ret;
	nlh->nlmsg_pid = getpid();

	ret = sendto(fd, nlh, nlh->nlmsg_len, 0, (struct sockaddr *)&dest_addr, sizeof(dest_addr));

	if (ret < 0) {
		printf("sendto(): %s\n", strerror(errno));
		close(fd);
		return -1;
	}
	return 0;
}

int recv_msg_netlink(int fd, struct nlmsghdr *nlh, int len)
{
	int ret, reply_len;

	nlh->nlmsg_len = NLMSG_SPACE(len);
	ret = recvfrom(fd, nlh_recv, nlh_recv->nlmsg_len, 0, NULL, NULL);
	if (ret < 0) {
		printf("recvfrom(): %s\n", strerror(errno));
		close(fd);
		return -1;
	}

	reply_len = ret - NLMSG_HDRLEN;
	return reply_len;
}
