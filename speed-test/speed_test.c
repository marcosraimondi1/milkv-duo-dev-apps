#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <linux/netlink.h>

#define MESSAGE_SIZE       4080 /* maximum payload size (rpmsg mtu) */
#define NETLINK_TEST       17
#define NUM_MESSAGES       2300
#define LOADING_BAR_LENGTH 50

void print_loading_bar(int progress);
double elapsed_secs(struct timeval start, struct timeval end);
void calculate_and_print_results(int msg_size, double time_taken, int num_messages);

int main(int argc, char *argv[])
{
	struct sockaddr_nl src_addr, dest_addr;
	struct nlmsghdr *nlh;
	int sock_fd, rc;
	struct timeval start, end;
	char send_buff[MESSAGE_SIZE];

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

	// Create netlink socket
	sock_fd = socket(PF_NETLINK, SOCK_RAW, NETLINK_TEST);
	if (sock_fd < 0) {
		printf("socket: %s\n", strerror(errno));
		return 1;
	}

	// Bind socket
	memset(&src_addr, 0, sizeof(src_addr));
	src_addr.nl_family = AF_NETLINK;
	src_addr.nl_pid = getpid();
	if (bind(sock_fd, (struct sockaddr *)&src_addr, sizeof(src_addr)) != 0) {
		printf("bind: %s\n", strerror(errno));
		close(sock_fd);
		return 1;
	}

	// Set destination address
	memset(&dest_addr, 0, sizeof(dest_addr));
	dest_addr.nl_family = AF_NETLINK;
	dest_addr.nl_pid = 0; // For Linux Kernel

	printf("\n------------------- TEST -------------------\n");
	printf("Size: %d bytes\n", MESSAGE_SIZE);
	printf("Number of messages: %d\n", num_msgs);

	// Allocate and initialize netlink message header
	nlh = (struct nlmsghdr *)malloc(NLMSG_SPACE(MESSAGE_SIZE));
	nlh->nlmsg_len = NLMSG_SPACE(MESSAGE_SIZE);
	nlh->nlmsg_pid = getpid();
	strcpy(NLMSG_DATA(nlh), send_buff);

	double total_time_secs = 0;
	for (int j = 0; j < num_msgs; j++) {

		gettimeofday(&start, NULL);

		// Send message
		rc = sendto(sock_fd, nlh, nlh->nlmsg_len, 0, (struct sockaddr *)&dest_addr,
			    sizeof(dest_addr));
		if (rc < 0) {
			printf("sendto(): %s\n", strerror(errno));
			close(sock_fd);
			return 1;
		}

		// Receive message
		rc = recvfrom(sock_fd, nlh, nlh->nlmsg_len, 0, NULL, NULL);
		if (rc < 0) {
			printf("recvfrom(): %s\n", strerror(errno));
			close(sock_fd);
			return 1;
		}

		gettimeofday(&end, NULL);
		total_time_secs += elapsed_secs(start, end);

		// Check received message
		char *recv_msg = (char *)NLMSG_DATA(nlh);
		int recv_len = nlh->nlmsg_len - NLMSG_HDRLEN;

		if (recv_len != MESSAGE_SIZE || memcmp(send_buff, recv_msg, MESSAGE_SIZE) != 0) {
			printf("Error: received message is different from sent message\n");
			close(sock_fd);
			return 1;
		}

		print_loading_bar((j + 1) * LOADING_BAR_LENGTH / num_msgs);

		nlh->nlmsg_len = NLMSG_SPACE(MESSAGE_SIZE);
		nlh->nlmsg_pid = getpid();
		strcpy(NLMSG_DATA(nlh), send_buff);
	}

	printf("\n");

	calculate_and_print_results(MESSAGE_SIZE, total_time_secs, num_msgs);

	free(nlh);
	close(sock_fd);
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
