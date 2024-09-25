#!/bin/python

from encoding import encode_board, decode_board

board = [[" ", "O", " "], [" ", "X", " "], [" ", " ", " "]]

encoded = encode_board(board)
print("encoded len", len(encoded))

filename = "python_encoded"

# Write the file
with open(filename, "wb") as f:
  f.write(encoded)

# Read the file
new_board = []
with open(filename, "rb") as f:
  new_board = decode_board(f.read())

print("Original python board")
print(board)
print("Python decoded message")
print(new_board)

# decode C encoded message
try:
  with open("c_encoded", "rb") as f:
    new_board = decode_board(f.read())

  print("C decoded message")
  print(new_board)
except Exception:
  pass
