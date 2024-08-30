#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <linux/netlink.h>

#define MESSAGE_SIZE       4080 /* maximum payload size (rpmsg mtu) */
#define NETLINK_ID         17
#define NUM_MESSAGES       2300
#define LOADING_BAR_LENGTH 50

struct sockaddr_nl src_addr, dest_addr;
struct nlmsghdr *nlh_send, *nlh_recv;

void print_loading_bar(int progress);
double elapsed_secs(struct timeval start, struct timeval end);
void calculate_and_print_results(int msg_size, double time_taken, int num_messages);

int rpmsg_init(void);
int rpmsg_exit(int fd);
int send_msg(int fd, char *msg, int len);
int recv_msg(int fd, int len, char *reply_msg, int *reply_len);

int main(int argc, char *argv[])
{
	int ret;
	int sock_fd, recv_len;
	struct timeval start, end;
	char send_buff[MESSAGE_SIZE];
	char recv_buff[MESSAGE_SIZE];

	int num_msgs = NUM_MESSAGES;
	if (argc > 1) {
		num_msgs = atoi(argv[1]);
		if (num_msgs <= 0) {
			printf("Invalid number of messages\n");
			return 1;
		}
	}

	// Initialize send buffer with 'a's
	memset(send_buff, 'a', MESSAGE_SIZE - 1);
	send_buff[MESSAGE_SIZE - 1] = '\0';

	sock_fd = rpmsg_init();

	if (sock_fd < 0) {
		return -1;
	}

	printf("\n------------------- TEST -------------------\n");
	printf("Size: %d bytes\n", MESSAGE_SIZE);
	printf("Number of messages: %d\n", num_msgs);

	double total_time_secs = 0;
	for (int j = 0; j < num_msgs; j++) {

		gettimeofday(&start, NULL);

		// Send message
		ret = send_msg(sock_fd, send_buff, MESSAGE_SIZE);
		if (ret < 0) {
			return -1;
		}

		// Receive message
		ret = recv_msg(sock_fd, MESSAGE_SIZE, recv_buff, &recv_len);
		if (ret < 0) {
			return -1;
		}

		gettimeofday(&end, NULL);
		total_time_secs += elapsed_secs(start, end);

		if (recv_len != MESSAGE_SIZE || memcmp(send_buff, recv_buff, MESSAGE_SIZE) != 0) {
			printf("Error: received message is different from sent message\n");
			close(sock_fd);
			return -1;
		}

		print_loading_bar((j + 1) * LOADING_BAR_LENGTH / num_msgs);
	}

	printf("\n");

	calculate_and_print_results(MESSAGE_SIZE, total_time_secs, num_msgs);

	rpmsg_exit(sock_fd);
	return 0;
}

int rpmsg_init()
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

	// Allocate and initialize netlink message header
	nlh_send = (struct nlmsghdr *)malloc(NLMSG_SPACE(MESSAGE_SIZE));
	nlh_send->nlmsg_len = NLMSG_SPACE(MESSAGE_SIZE);
	nlh_send->nlmsg_pid = getpid();

	nlh_recv = (struct nlmsghdr *)malloc(NLMSG_SPACE(MESSAGE_SIZE));

	return sock_fd;
}

int rpmsg_exit(int fd)
{
	free(nlh_send);
	free(nlh_recv);
	close(fd);
	return 0;
}

int send_msg(int fd, char *msg, int len)
{
	int ret;
	strcpy(NLMSG_DATA(nlh_send), msg); // this adds latency
	nlh_send->nlmsg_len = NLMSG_SPACE(len);
	nlh_send->nlmsg_pid = getpid();

	ret = sendto(fd, nlh_send, nlh_send->nlmsg_len, 0, (struct sockaddr *)&dest_addr,
		     sizeof(dest_addr));
	if (ret < 0) {
		printf("sendto(): %s\n", strerror(errno));
		close(fd);
		return -1;
	}
	return 0;
}

int recv_msg(int fd, int len, char *reply_msg, int *reply_len)
{
	int ret;
	nlh_recv->nlmsg_len = NLMSG_SPACE(len);
	ret = recvfrom(fd, nlh_recv, nlh_recv->nlmsg_len, 0, NULL, NULL);
	if (ret < 0) {
		printf("recvfrom(): %s\n", strerror(errno));
		close(fd);
		return -1;
	}

	char *msg = (char *)NLMSG_DATA(nlh_recv);
	*reply_len = nlh_recv->nlmsg_len - NLMSG_HDRLEN;
	memcpy(reply_msg, msg, *reply_len); // this adds latency
	return 0;
}

double elapsed_secs(struct timeval start, struct timeval end)
{
	return (end.tv_sec - start.tv_sec) + (end.tv_usec - start.tv_usec) * 1e-6;
}

void calculate_and_print_results(int msg_size, double time_taken_secs, int num_messages)
{
	double latency = time_taken_secs / num_messages;
	double throughput = (msg_size * num_messages) / time_taken_secs;

	printf("Latency: %.6f seconds per message\n", latency);
	printf("Time taken: %.6f seconds\n", time_taken_secs);
	printf("Throughput: %.6f MB/s\n", throughput / 1e6);
}

void print_loading_bar(int progress)
{
	printf("\r[");
	printf("\033[42m");

	for (int i = 0; i < LOADING_BAR_LENGTH; i++) {
		if (i < progress) {
			printf(" ");
		} else {
			printf("\033[0m");
			printf(" ");
		}
	}

	printf("\033[0m");
	printf("] %d%%", progress * 2);
	fflush(stdout);
}
