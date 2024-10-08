from encoding import decode_msg
from comms import Connection, Message
import time

sock = Connection(nlservice=19)

def send_msg():
    msg = Message(payload="hello".encode())
    sock.send(msg)

def receive_msg():
    encoded = sock.recve()
    return decode_msg(encoded)


while True:
    send_msg()
    receive_msg()
    time.sleep(5)
