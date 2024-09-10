#include <sys/select.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <stdint.h>
#include <stddef.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <time.h>
#include <stdbool.h>
#include <semaphore.h>
#include <linux/rpmsg.h>
#include <sys/socket.h>
#include <linux/netlink.h>
#include <errno.h>

#define NETLINK_ID 17

#define NUM_ITERATIONS_DEFAULT 10

/* message size options */
#define RPMSG_BUFFER_SIZE 512 // as defined in the kernel (includes header)
#define MSG_SIZE_DEFAULT  (RPMSG_BUFFER_SIZE - 16)

struct sockaddr_nl src_addr, dest_addr;
struct nlmsghdr *nlh_send, *nlh_recv;

int rpmsg_init_netlink(void);
int rpmsg_exit_netlink(int fd);
int send_msg_netlink(int fd, struct nlmsghdr *nlh, struct sockaddr_nl dest_addr);
int recv_msg_netlink(int fd, struct nlmsghdr *nlh, int len);

int run_app(void);

void usage(void);

int main(int argc, char *argv[])
{
	int status = run_app();
	if (status < 0) {
		printf("APP STATUS: FAILED\n");
	} else {
		printf("APP STATUS: PASSED\n");
	}

	return 0;
}

int run_app()
{
	int fd = rpmsg_init_netlink();

	if (fd < 0) {
		printf("rpmsg_init failed\n");
		return -1;
	}

	float packet_send_buf[4] = {1.1, 2.2, 3.3, 4.4};
	float *packet_recv_buf;

	int packet_len = sizeof(packet_send_buf);

	memcpy(NLMSG_DATA(nlh_send), packet_send_buf, sizeof(packet_send_buf));
	nlh_send->nlmsg_len = NLMSG_SPACE(packet_len);

	int ret = send_msg_netlink(fd, nlh_send, dest_addr);
	if (ret < 0) {
		printf("send_msg failed for iteration ret = %d\n", ret);
		goto out;
	}
	ret = recv_msg_netlink(fd, nlh_recv, packet_len);

	packet_recv_buf = NLMSG_DATA(nlh_recv);
	if (ret < 0) {
		printf("recv_msg failed for iteration ret = %d\n", ret);
		goto out;
	}
	if (ret != packet_len) {
		printf("bytes written does not match received, sent = %d, recv = %d\n", packet_len,
		       ret);
		goto out;
	}

	// print results
	for (int i = 0; i < 4; i++) {
		printf("x: %f, y: %f\n", packet_send_buf[i], packet_recv_buf[i]);
	}

out:
	rpmsg_exit_netlink(fd);
	return ret;
}

int rpmsg_init_netlink()
{
	int sock_fd;

	// Create netlink socket
	sock_fd = socket(PF_NETLINK, SOCK_RAW, NETLINK_ID);
	if (sock_fd < 0) {
		printf("socket: %s\n", strerror(errno));
		return -1;
	}

	// Bind socket
	memset(&src_addr, 0, sizeof(src_addr));
	src_addr.nl_family = AF_NETLINK;
	src_addr.nl_pid = getpid();
	if (bind(sock_fd, (struct sockaddr *)&src_addr, sizeof(src_addr)) != 0) {
		printf("bind: %s\n", strerror(errno));
		close(sock_fd);
		return -1;
	}

	// Set destination address
	memset(&dest_addr, 0, sizeof(dest_addr));
	dest_addr.nl_family = AF_NETLINK;
	dest_addr.nl_pid = 0; // For Linux Kernel

	// Allocate netlink message header
	nlh_send = (struct nlmsghdr *)malloc(NLMSG_SPACE(RPMSG_BUFFER_SIZE - 16));
	nlh_recv = (struct nlmsghdr *)malloc(NLMSG_SPACE(RPMSG_BUFFER_SIZE - 16));

	return sock_fd;
}

int rpmsg_exit_netlink(int fd)
{
	free(nlh_send);
	free(nlh_recv);
	close(fd);
	return 0;
}

int send_msg_netlink(int fd, struct nlmsghdr *nlh, struct sockaddr_nl dest_addr)
{
	int ret;
	nlh->nlmsg_pid = getpid();

	ret = sendto(fd, nlh, nlh->nlmsg_len, 0, (struct sockaddr *)&dest_addr, sizeof(dest_addr));

	if (ret < 0) {
		printf("sendto(): %s\n", strerror(errno));
		close(fd);
		return -1;
	}
	return 0;
}

int recv_msg_netlink(int fd, struct nlmsghdr *nlh, int len)
{
	int ret, reply_len;

	nlh->nlmsg_len = NLMSG_SPACE(len);
	ret = recvfrom(fd, nlh_recv, nlh_recv->nlmsg_len, 0, NULL, NULL);
	if (ret < 0) {
		printf("recvfrom(): %s\n", strerror(errno));
		close(fd);
		return -1;
	}

	reply_len = ret - NLMSG_HDRLEN;
	return reply_len;
}

void usage()
{
	printf("Usage: rpmsg_char_simple [-n <num_msgs>] [-s <msg_size>] \n");
	printf("\t\tDefaults: num_msgs: %d msg_size: %d\n", NUM_ITERATIONS_DEFAULT,
	       MSG_SIZE_DEFAULT);
}
