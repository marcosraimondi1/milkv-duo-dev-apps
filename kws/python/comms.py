import os
import socket
import struct

# constants
NLMSG_HDRLEN = 16 # header length

# types
NLMSG_NOOP = 1
NLMSG_ERROR = 2
NLMSG_DONE = 3
NLMSG_OVERRUN = 4
MSG_SETCFG = 11
MSG_GETCFG = 12
NLMSG_MIN_TYPE = 0x10

# flags
NLM_F_REQUEST = 1
NLM_F_MULTI = 2
NLM_F_ACK = 4
NLM_F_ECHO = 8

class Message:
    def __init__(self, msg_type=NLMSG_DONE, flags=0, seq=-1, payload=None):
        self.type = msg_type
        self.flags = flags
        self.seq = seq
        self.pid = 1
        payload = payload or bytes()
        self.payload = payload

class Connection(object):
    """
    Object representing Netlink socket connection to the kernel.
    """
    def __init__(self, nlservice=31, groups=0):
        # nlservice = Netlink IP service
        self.fd = socket.socket(socket.AF_NETLINK, socket.SOCK_RAW, nlservice)
        self.fd.setsockopt(socket.SOL_SOCKET, socket.SO_SNDBUF, 65536)
        self.fd.setsockopt(socket.SOL_SOCKET, socket.SO_RCVBUF, 65536)
        self.fd.bind((0, groups)) # pid=0 lets kernel assign socket PID
        self.pid, self.groups = self.fd.getsockname()
        self.pid = os.getpid()
        self._seq = 0
        
    def send(self, msg):
        if isinstance(msg, Message):
            if msg.seq == -1: 
                msg.seq = self.seq()
            #msg.seq = 1
            msg.pid = self.pid
            length = len(msg.payload)
            hdr = struct.pack("IHHII", length + 4 * 4, msg.type,
                          msg.flags, msg.seq, msg.pid) 

            msg = hdr + msg.payload

            return self.fd.send(msg)
     
    def recve(self):
        data = self.fd.recv(16384)
        _, msg_type, flags, seq, pid = struct.unpack("IHHII", data[:NLMSG_HDRLEN])
        nlh_len = struct.unpack("I", data[0:4])[0]

        msg = Message(msg_type, flags, seq, data[NLMSG_HDRLEN:nlh_len])

        msg.pid = pid
        if msg.type == NLMSG_ERROR:
            errno = -struct.unpack("i", msg.payload[:4])[0]
            if errno != 0:
                err = OSError("Netlink error: %s (%d)" % (
                                                    os.strerror(errno), errno))
                err.errno = errno
                print("err :",err)
                raise err
        
        return msg.payload 
        
    def seq(self):
        self._seq += 1
        return self._seq

