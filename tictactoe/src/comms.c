#include "comms.h"
#include <linux/netlink.h>
#include <stdlib.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

static struct sockaddr_nl src_addr, dest_addr;
static struct nlmsghdr *nlh_send = NULL, *nlh_recv = NULL;
static int sock_fd;

int rpmsg_init_netlink()
{
	// Create netlink socket
	sock_fd = socket(PF_NETLINK, SOCK_RAW, NETLINK_ID);
	if (sock_fd < 0) {
		printf("socket: %s\n", strerror(errno));
		return ERROR;
	}

	// Bind socket
	memset(&src_addr, 0, sizeof(src_addr));
	src_addr.nl_family = AF_NETLINK;
	src_addr.nl_pid = getpid();
	if (bind(sock_fd, (struct sockaddr *)&src_addr, sizeof(src_addr)) != 0) {
		printf("bind: %s\n", strerror(errno));
		close(sock_fd);
		return ERROR;
	}

	// Set destination address
	memset(&dest_addr, 0, sizeof(dest_addr));
	dest_addr.nl_family = AF_NETLINK;
	dest_addr.nl_pid = 0; // For Linux Kernel

	// Allocate netlink message header
	nlh_send = (struct nlmsghdr *)malloc(NLMSG_SPACE(MAX_DATA_SIZE));
	nlh_recv = (struct nlmsghdr *)malloc(NLMSG_SPACE(MAX_DATA_SIZE));

	return SUCCESS;
}

int rpmsg_exit_netlink(void)
{
	free(nlh_send);
	free(nlh_recv);
	close(sock_fd);
	return SUCCESS;
}

static int send_msg_netlink(void)
{
	int ret;
	nlh_send->nlmsg_pid = getpid();

	ret = sendto(sock_fd, nlh_send, nlh_send->nlmsg_len, 0, (struct sockaddr *)&dest_addr,
		     sizeof(dest_addr));

	if (ret < 0) {
		printf("sendto(): %s\n", strerror(errno));
		return ERROR;
	}
	return SUCCESS;
}

static int recv_msg_netlink(int len)
{
	int ret, reply_len;

	nlh_recv->nlmsg_len = NLMSG_SPACE(len);
	ret = recvfrom(sock_fd, nlh_recv, nlh_recv->nlmsg_len, 0, NULL, NULL);
	if (ret < 0) {
		printf("recvfrom(): %s\n", strerror(errno));
		return ERROR;
	}

	reply_len = ret - NLMSG_HDRLEN;
	return reply_len;
}

int send_msg(void *msg, size_t msg_size)
{
	if (msg_size > MAX_DATA_SIZE) {
		printf("Message size too large\n");
		return ERROR;
	}
	memcpy(NLMSG_DATA(nlh_send), msg, msg_size);
	nlh_send->nlmsg_len = NLMSG_SPACE(msg_size);
	int ret = send_msg_netlink();
	if (ret < 0) {
		printf("send_msg failed ret = %d\n", ret);
		return ERROR;
	}
	return SUCCESS;
}

int recv_msg(void *msg, size_t msg_size)
{
	if (msg_size > MAX_DATA_SIZE) {
		printf("Message size too large\n");
		return ERROR;
	}
	int ret = recv_msg_netlink(msg_size);
	if (ret < 0) {
		printf("recv_msg failed ret = %d\n", ret);
		return ERROR;
	}
	memcpy(msg, NLMSG_DATA(nlh_recv), msg_size);

	return ret;
}

/**
 * @brief Draw the board
 * @param board The current board
 */
void draw(char board[3][3])
{
	printf("\033[H\033[J"); // clear screen
	printf("\t0\t1\t2\n\n");
	for (int i = 0; i < 3; i++) {
		printf("%d", i);
		for (int j = 0; j < 3; j++) {
			printf("\t%c", board[i][j]);
		}
		printf("\n\n");
	}
}
