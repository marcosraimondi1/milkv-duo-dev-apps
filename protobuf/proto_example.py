#!/bin/python

from message_pb2 import Board

board = Board()

row0 = board.rows.add()
row0.marks.extend([board.Mark.MARK_X, board.Mark.MARK_O, board.Mark.MARK_X])
row1 = board.rows.add()
row1.marks.extend([board.Mark.MARK_O, board.Mark.MARK_X, board.Mark.MARK_O])
row2 = board.rows.add()
row2.marks.extend([board.Mark.MARK_X, board.Mark.MARK_O, board.Mark.MARK_X])

filename = "python_encoded"
# Write the file
with open(filename, "wb") as f:
  f.write(board.SerializeToString())

# Read the file
new_board = Board()
with open(filename, "rb") as f:
  new_board.ParseFromString(f.read())

# check if the new board is the same as the old one
if board != new_board:
  print("Something went wrong")

def mark_to_char(mark):
  if mark == board.Mark.MARK_X:
    return "X"
  elif mark == board.Mark.MARK_O:
    return "O"
  else:
    return " "

i = 0
for row in new_board.rows:
  j = 0
  for mark in row.marks:
    print(f"{mark_to_char(board.rows[i].marks[j])}={mark_to_char(mark)}", end=" ")
    j += 1
  i += 1
  print("")


# decode C encoded message
try:
  with open("c_encoded", "rb") as f:
    new_board.ParseFromString(f.read())

  print("C encoded message")
  print(new_board)
except Exception:
  print("")
