"""
Tic Tac Toe Player
"""

import math
import copy

X = "X"
O = "O"
EMPTY = " "


def initial_state():
    """
    Returns starting state of the board.
    """
    return [[EMPTY, EMPTY, EMPTY],
            [EMPTY, EMPTY, EMPTY],
            [EMPTY, EMPTY, EMPTY]]


def player(board):
    """
    Returns player who has the next turn on a board.
    initial game state, x gets the first move
    """
    init_board = initial_state()

    if board == init_board:
        return X

    x_count = 0
    o_count = 0
    for i in range(3):
        for j in range(3):
            if board[i][j] == X:
                x_count += 1
            elif board[i][j] == O:
                o_count += 1

    if x_count == o_count:
        return X
    elif x_count > o_count and x_count != 5:
        return O
    else:
        return None


def actions(board):
    """
    Returns set of all possible actions (i, j) available on the board.
    possible moves are any cells that do not already haven't been used

    Each action should be represented as a tuple (i, j)
    """

    actions = []

    for i in range(3):
        for j in range(3):
            if board[i][j] == EMPTY:
                actions.append((i, j))

    if len(actions) == 0:
        return None

    return actions


def result(board, action):
    """
    Returns the board that results from making move (i, j) on the board.
    returns a new board state without modifying the original board (deep copy)

    """
    new_board = copy.deepcopy(board)
    try:
        new_board[action[0]][action[1]] = player(board)
    except:
        print(action, new_board)

    return new_board


def winner(board):
    """
    Returns the winner of the game, if there is one.
    O wins return o
    X wins return x
    No winner return None
    """
    for i in range(3):
        if board[i][0] == board[i][1] and board[i][0] == board[i][2] and board[i][0] != EMPTY:
            return board[i][0]

    for i in range(3):
        if board[0][i] == board[1][i] and board[0][i] == board[2][i] and board[0][i] != EMPTY:
            return board[0][i]

    if board[0][0] == board[1][1] and board[0][0] == board[2][2] and board[0][0] != EMPTY:
        return board[0][0]

    elif board[0][2] == board[1][1] and board[0][2] == board[2][0] and board[0][2] != EMPTY:
        return board[0][2]

    else:
        return None


def terminal(board):
    """
    Returns True if game is over, False otherwise.
    """
    if winner(board) == X or winner(board) == O:
        return True

    x_count = 0
    for i in range(3):
        x_count += board[i].count(X)

    if x_count == 5:
        return True

    else:
        return False


def utility(board):
    """
    Returns 1 if X has won the game, -1 if O has won, 0 otherwise.
    call winner function
    """
    winer = winner(board)
    if winer == X:
        return 1
    elif winer == O:
        return -1
    else:
        return 0


def Max_Value(board):

    if terminal(board):
        return utility(board)

    v = - math.inf

    for action in actions(board):
        v = max(v, Min_Value(result(board, action)))

    return v


def Min_Value(board):

    if terminal(board):
        return utility(board)

    v = math.inf

    for action in actions(board):
        v = min(v, Max_Value(result(board, action)))

    return v


def minimax(board):
    """
    Returns the optimal action for the current player on the board.
    if board = terminal board return None
    return optimal action from available actions
    """
    if terminal(board):
        return None

    if player(board) == X:
        val = Max_Value(board)
        for action in actions(board):
            if Min_Value(result(board, action)) == val:
                return action

    elif player(board) == O:
        val = Min_Value(board)
        for action in actions(board):
            if Max_Value(result(board, action)) == val:
                return action
