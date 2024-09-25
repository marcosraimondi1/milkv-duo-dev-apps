from comms import Connection, Message
from encoding import encode_board, decode_board

sock = Connection(nlservice=17)

board = [[" ", "O", " "], [" ", "X", " "], [" ", " ", " "]]
encoded = encode_board(board)

msg1 = Message(payload=encoded) 

print(f"Sending {len(encoded)} bytes to kernel...")
res1 = sock.send(msg1)

print("waiting for kernel ..")
res2 = sock.recve()
print(f"{len(res2)} bytes received from kernel")
new_board = decode_board(res2)

print(board)
print(new_board)

