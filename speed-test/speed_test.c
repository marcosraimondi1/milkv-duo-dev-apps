#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <linux/netlink.h>

#define MESSAGE_SIZE       4080 /* maximum payload size (rpmsg mtu)*/
#define NETLINK_TEST       17
#define NUM_MESSAGES       2300
#define LOADING_BAR_LENGTH 50

void print_loading_bar(int progress);
double elapsed_secs(struct timeval start, struct timeval end);
void calculate_and_print_results(int msg_size, double time_taken, int num_messages);

int main()
{
	struct sockaddr_nl src_addr, dest_addr;
	struct nlmsghdr *nlh;
	struct msghdr msg;
	struct iovec iov;
	int sock_fd, rc;
	struct timeval start, end;
	char send_buff[MESSAGE_SIZE];

	for (int i = 0; i < MESSAGE_SIZE; i++) {
		send_buff[i] = 'a';
	}
	send_buff[MESSAGE_SIZE - 1] = '\0';

	sock_fd = socket(PF_NETLINK, SOCK_RAW, NETLINK_TEST);
	if (sock_fd < 0) {
		printf("socket: %s\n", strerror(errno));
		return 1;
	}

	memset(&src_addr, 0, sizeof(src_addr));
	src_addr.nl_family = AF_NETLINK;
	src_addr.nl_pid = getpid();
	src_addr.nl_groups = 0;
	if (bind(sock_fd, (struct sockaddr *)&src_addr, sizeof(src_addr)) != 0) {
		printf("bind: %s\n", strerror(errno));
		close(sock_fd);
		return 1;
	}

	memset(&dest_addr, 0, sizeof(dest_addr));
	dest_addr.nl_family = AF_NETLINK;
	dest_addr.nl_pid = 0; // For Linux Kernel
	dest_addr.nl_groups = 0;

	printf("\n------------------- TEST -------------------\n");
	printf("Size: %d bytes\n", MESSAGE_SIZE);
	printf("Number of messages: %d\n", NUM_MESSAGES);

	nlh = (struct nlmsghdr *)malloc(NLMSG_SPACE(MESSAGE_SIZE));
	nlh->nlmsg_len = NLMSG_SPACE(MESSAGE_SIZE);
	nlh->nlmsg_pid = getpid();
	nlh->nlmsg_flags = 0;

	memset(send_buff, 'a', MESSAGE_SIZE - 1);
	send_buff[MESSAGE_SIZE - 1] = '\0';
	strcpy(NLMSG_DATA(nlh), send_buff);

	memset(&iov, 0, sizeof(iov));
	iov.iov_base = (void *)nlh;
	iov.iov_len = nlh->nlmsg_len;

	memset(&msg, 0, sizeof(msg));
	msg.msg_name = (void *)&dest_addr;
	msg.msg_namelen = sizeof(dest_addr);
	msg.msg_iov = &iov;
	msg.msg_iovlen = 1;

	double total_time_secs = 0;
	for (int j = 0; j < NUM_MESSAGES; j++) {

		gettimeofday(&start, NULL);
		rc = sendmsg(sock_fd, &msg, 0);
		if (rc < 0) {
			printf("sendmsg(): %s\n", strerror(errno));
			close(sock_fd);
			return 1;
		}

		rc = recvmsg(sock_fd, &msg, 0);
		if (rc < 0) {
			printf("recvmsg(): %s\n", strerror(errno));
			close(sock_fd);
			return 1;
		}
		gettimeofday(&end, NULL);

		total_time_secs += elapsed_secs(start, end);

		// print received message
		char *recv_msg = (char *)NLMSG_DATA(nlh);

		// received len
		int recv_len = nlh->nlmsg_len - NLMSG_HDRLEN;

		// compare received message with sent message
		if (recv_len != MESSAGE_SIZE || memcmp(send_buff, recv_msg, MESSAGE_SIZE) != 0) {
			printf("Error: received message is different from sent message\n");
			printf("Received: %s of len %d\n", recv_msg, recv_len);
			close(sock_fd);
			return 1;
		}

		for (int i = 0; i < MESSAGE_SIZE; i++) {
			send_buff[i] = 'a';
		}
		send_buff[MESSAGE_SIZE - 1] = '\0';
		strcpy(NLMSG_DATA(nlh), send_buff);
		nlh->nlmsg_pid = getpid();
		nlh->nlmsg_len = NLMSG_SPACE(MESSAGE_SIZE);
		nlh->nlmsg_flags = 0;

		print_loading_bar((j + 1) * LOADING_BAR_LENGTH / NUM_MESSAGES);
	}

	printf("\n");

	calculate_and_print_results(MESSAGE_SIZE, total_time_secs, NUM_MESSAGES);

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

	// Throughput = number of messages * message size / time taken
	double throughput = (msg_size * num_messages) / (time_taken_secs); // Bytes per second

	printf("Latency: %.6f seconds per message\n", latency);
	printf("Time taken: %.6f seconds\n", time_taken_secs);
	printf("Throughput: %.6f MB/s\n", throughput / 1e6);
}

void print_loading_bar(int progress)
{
	printf("\r["); // Return to the start of the line and print the opening bracket

	printf("\033[42m"); // green

	for (int i = 0; i < LOADING_BAR_LENGTH; i++) {
		if (i < progress) {
			printf(" "); // Print the | character to represent progress
		} else {
			printf("\033[0m"); // normal
			printf(" ");       // Print a space for the remaining part of the bar
		}
	}

	printf("\033[0m");              // normal
	printf("] %d%%", progress * 2); // Print the percentage completed
	fflush(stdout);                 // Ensure the output is immediately displayed
}
