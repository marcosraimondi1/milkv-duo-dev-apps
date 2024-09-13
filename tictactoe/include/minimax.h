#ifndef MINIMAX_H
#define MINIMAX_H

#define X     'X'
#define O     'O'
#define EMPTY ' '

struct action {
	int row;
	int col;
};

void initial_state(char board[3][3]);
char player(char board[3][3]);
int actions(char board[3][3], struct action action_list[9]);
void result(char new_board[3][3], char board[3][3], struct action action);
char winner(char board[3][3]);
int terminal(char board[3][3]);
int utility(char board[3][3]);
int Max_Value(char board[3][3]);
int Min_Value(char board[3][3]);
void minimax(char board[3][3], struct action *best_move);
void draw(char board[3][3]);

#endif // !MINIMAX_H
