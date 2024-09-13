#ifndef COMMS_H
#define COMMS_H

#include <sys/socket.h>

#define NETLINK_ID        18  // as defined in the kernel module
#define RPMSG_BUFFER_SIZE 512 // as defined in the kernel (includes header)
#define MAX_DATA_SIZE     RPMSG_BUFFER_SIZE - 16
#define SUCCESS           0
#define ERROR             -1

int rpmsg_init_netlink(void);
int rpmsg_exit_netlink(void);
static int send_msg_netlink(void);
static int recv_msg_netlink(int len);
int send_msg(void *msg, size_t msg_size);
int recv_msg(void *msg, size_t msg_size);

#endif
