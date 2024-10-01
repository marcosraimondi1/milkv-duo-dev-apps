from encoding import encode_board, decode_board
from comms import Connection, Message
import tictactoe as ttt
import time

sock = Connection(nlservice=18)

def send_board(board):
    encoded = encode_board(board)
    msg = Message(payload=encoded)
    sock.send(msg)

def receive_board():
    encoded = sock.recve()
    return decode_board(encoded)

user = None
auto = False
board = ttt.initial_state()
ai_turn = False

while True:
    # clear screen
    print("\033[H\033[J")

    # Let user choose a player.
    if user is None:

        # Draw title
        print("Play Tic-Tac-Toe")

        auto = input("Auto mode? (y/n): ") == "y"

        while user is None:
            option = input("Choose X or O: ")
            if option.upper() == "X":
                user = ttt.X
            elif option.upper() == "O":
                user = ttt.O

    else:

        # Draw game board
        print("\t 0\t 1\t 2\n")
        for i in range(3):
            print(i, end="")
            for j in range(3):
                print("\t", board[i][j], end="")
            print("\n")

        game_over = ttt.terminal(board)
        player = ttt.player(board)

        # Show title
        if game_over:
            winner = ttt.winner(board)
            if winner is None:
                title = f"Game Over: Tie."
            else:
                title = f"Game Over: {winner} wins."
        elif user == player:
            title = f"Play as {user}"
        else:
            title = f"Computer thinking..."

        print(title)

        # Check for AI move
        if user != player and not game_over:
            if ai_turn:
                if (auto):
                    time.sleep(1)
                send_board(board) # Send board to AI
                board = receive_board() # Receive board from AI
                ai_turn = False
            else:
                ai_turn = True

        # Check for a user move
        if user == player and not game_over:
            move = None
            if auto:
                # Auto mode
                time.sleep(1)
                move = ttt.minimax(board)
            else:
                while move is None:
                    move = input("Enter move (row col): ").split(" ")
                    try:
                        move = (int(move[0]), int(move[1]))
                        if board[move[0]][move[1]] != ttt.EMPTY:
                            move = None
                            print("Invalid move.")
                    except:
                        move = None
                        print("Invalid move.")

            board = ttt.result(board, move)

        if game_over:
            exit()


