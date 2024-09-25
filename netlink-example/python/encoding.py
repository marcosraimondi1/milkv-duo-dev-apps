from typing import List
from message_pb2 import Board

EMPTY = Board.Mark.MARK_EMPTY
X = Board.Mark.MARK_X
O = Board.Mark.MARK_O

def mark_to_char(mark) -> str:
  if mark == X:
    return "X"
  elif mark == O:
    return "O"
  else:
    return " "

def marks_to_chars(marks:List) -> List[str]:
  return [mark_to_char(mark) for mark in marks]

def char_to_mark(char:str):
  if char == "X":
    return X
  elif char == "O":
    return O
  else:
    return EMPTY

def chars_to_marks(chars:List[str]):
  return [char_to_mark(char) for char in chars]

def encode_board(board_data:List[List[str]]) -> bytes:
  board = Board()
  board.rows.add().marks[:] = chars_to_marks(board_data[0])
  board.rows.add().marks[:] = chars_to_marks(board_data[1])
  board.rows.add().marks[:] = chars_to_marks(board_data[2])

  return board.SerializeToString()

def decode_board(data:bytes) -> List[List[str]]:
  board = Board()
  board.ParseFromString(data)

  return [marks_to_chars(row.marks) for row in board.rows]



