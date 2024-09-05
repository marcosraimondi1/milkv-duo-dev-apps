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
#include "payload.h"

#define NETLINK_ID 17

#define NUM_ITERATIONS_DEFAULT 100
/*
 * This test can measure round-trip latencies up to 20 ms
 * Latencies measured in microseconds (us)
 */
#define LATENCY_RANGE          20000

/* message size options */
#define RPMSG_BUFFER_SIZE 512 // as defined in the kernel (includes header)
#define MSG_SIZE_DEFAULT  (RPMSG_BUFFER_SIZE - 16)

struct sockaddr_nl src_addr, dest_addr;
struct nlmsghdr *nlh_send, *nlh_recv;

int rpmsg_init_netlink(void);
int rpmsg_exit_netlink(int fd);
int send_msg_netlink(int fd, struct nlmsghdr *nlh, struct sockaddr_nl dest_addr);
int recv_msg_netlink(int fd, struct nlmsghdr *nlh, int len);

int rpmsg_char_ping(int num_msgs, int msg_size);

int check_payload(const char *sent, const char *recv, int packet_len);
void usage(void);

int main(int argc, char *argv[])
{
	int ret, status, c;
	int num_msgs = NUM_ITERATIONS_DEFAULT;
	int msg_size = MSG_SIZE_DEFAULT;

	while (1) {
		c = getopt(argc, argv, "n:s:");
		if (c == -1) {
			break;
		}

		switch (c) {
		case 's':
			msg_size = atoi(optarg);
			break;
		case 'n':
			num_msgs = atoi(optarg);
			break;
		default:
			usage();
			exit(0);
		}
	}

	status = rpmsg_char_ping(num_msgs, msg_size);

	if (status < 0) {
		printf("TEST STATUS: FAILED\n");
	} else {
		printf("TEST STATUS: PASSED\n");
	}

	return 0;
}

/* single thread communicating with a single endpoint */
int rpmsg_char_ping(int num_msgs, int msg_size)
{
	int ret = 0;
	int i = 0;
	int packet_len;
	int flags = 0;
	/*
	 * Each RPMsg packet can have up to 496 bytes of data:
	 * 512 bytes total - 16 byte header = 496
	 */
	char packet_send_buf[msg_size];
	char *packet_recv_buf;

	/*
	 * Variables used for latency benchmarks
	 */
	struct timespec ts_current;
	struct timespec ts_end;
	struct timespec ts_start_test;
	struct timespec ts_end_test;

	/* latency measured in us */
	int latency = 0;
	int latencies[LATENCY_RANGE] = {0}; // histogram of latencies
	int latency_worst_case = 0;
	double latency_average =
		0; /* try double, since long long might have overflowed w/ 1Billion+ iterations */
	FILE *file_ptr;

	int fd = rpmsg_init_netlink();

	if (fd < 0) {
		printf("rpmsg_init failed\n");
		return -1;
	}

	memset(packet_send_buf, 0, sizeof(packet_send_buf));

	// snprintf adds null terminator to the end of the string
	snprintf(packet_send_buf, msg_size, STRING_PAYLOAD);

	// strlen does not include null terminator
	packet_len = strlen(packet_send_buf) + 1;
	if (packet_len != msg_size) {
		printf("Payload size is different to packet_len\n");
		return -1;
	}

	// send init test
	strcpy(NLMSG_DATA(nlh_send), "init");
	nlh_send->nlmsg_len = NLMSG_SPACE(5);
	ret = send_msg_netlink(fd, nlh_send, dest_addr);
	if (ret < 0) {
		printf("send_msg failed for start, ret = %d\n", i);
		goto out;
	}

	strcpy(NLMSG_DATA(nlh_send), packet_send_buf);
	nlh_send->nlmsg_len = NLMSG_SPACE(packet_len);

	/* double-check: is packet_len changing, or fixed at 496? */
	// printf("packet_len = %d\n", packet_len);

	/* remove prints to speed up the test execution time */
	// printf("Sending message #%d: %s\n", i, packet_send_buf);

	printf("Starting test with %d messages of size %d bytes\n", num_msgs, packet_len);

	clock_gettime(CLOCK_MONOTONIC, &ts_start_test);
	for (i = 0; i < num_msgs; i++) {

		clock_gettime(CLOCK_MONOTONIC, &ts_current);

		ret = send_msg_netlink(fd, nlh_send, dest_addr);
		if (ret < 0) {
			printf("send_msg failed for iteration %d, ret = %d\n", i, ret);
			goto out;
		}

		ret = recv_msg_netlink(fd, nlh_recv, packet_len);
		packet_recv_buf = NLMSG_DATA(nlh_recv);
		if (ret < 0) {
			printf("recv_msg failed for iteration %d, ret = %d\n", i, ret);
			goto out;
		}
		if (ret != packet_len) {
			printf("bytes written does not match received, sent = %d, recv = %d\n",
			       packet_len, ret);
			printf("packet_send_buf = %s\n", packet_send_buf);
			printf("packet_recv_buf = %s\n", packet_recv_buf);
			goto out;
		}
		clock_gettime(CLOCK_MONOTONIC, &ts_end);

		if (check_payload(packet_send_buf, packet_recv_buf, packet_len) < 0) {
			ret = -1;
			goto out;
		}

		/* latency measured in usec */
		latency = (ts_end.tv_nsec - ts_current.tv_nsec) / 1000;

		/* if latency is greater than LATENCY_RANGE, throw an error and exit */
		if (latency > LATENCY_RANGE) {
			printf("latency is too large to be recorded: %d usec\n", latency);
			goto out;
		}

		/* increment the counter for that specific latency measurement */
		latencies[latency]++;

		/* remove prints to speed up the test execution time */
		// printf("Received message #%d: round trip delay(usecs) = %ld\n", i,(ts_end.tv_nsec
		// - ts_current.tv_nsec)/1000); printf("%s\n", packet_buf);
	}

	clock_gettime(CLOCK_MONOTONIC, &ts_end_test);

	// end test
	strcpy(NLMSG_DATA(nlh_send), "end");
	nlh_send->nlmsg_len = NLMSG_SPACE(4);
	ret = send_msg_netlink(fd, nlh_send, dest_addr);
	if (ret < 0) {
		printf("send_msg failed for end, ret = %d\n", ret);
		goto out;
	}

	/* find worst-case latency */
	for (i = LATENCY_RANGE - 1; i > 0; i--) {
		if (latencies[i] != 0) {
			latency_worst_case = i;
			break;
		}
	}

	/* WARNING: The average latency calculation is currently being validated */
	/* find the average latency */
	for (i = LATENCY_RANGE - 1; i > 0; i--) {
		/* e.g., if latencies[60] = 17, that means there was a latency of 60us 17 times */
		latency_average = latency_average + (latencies[i] * i) / num_msgs;
	}
	/* old code from using long long instead of double */
	/*latency_average = latency_average / num_msgs;*/

	/* export the latency measurements to a file */
	file_ptr = fopen("histogram.txt", "w");

	fprintf(file_ptr, "latency [us], repetitions\n");
	for (unsigned int i = 0; i < LATENCY_RANGE; i++) {
		fprintf(file_ptr, "%d , ", i);
		fprintf(file_ptr, "%d", latencies[i]);
		fprintf(file_ptr, "\n");
	}
	fclose(file_ptr);

	printf("\nCommunicated %d messages successfully\n\n", num_msgs);
	printf("Total execution time for the test: %ld seconds\n",
	       ts_end_test.tv_sec - ts_start_test.tv_sec);
	printf("Average round-trip latency: %f [us]\n", latency_average);
	printf("Worst-case round-trip latency: %d [us]\n", latency_worst_case);
	printf("Histogram data at histogram.txt\n");

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

int check_payload(const char *sent, const char *recv, int packet_len)
{
	if (strncmp(sent, recv, packet_len) != 0) {
		printf("Payloads do not match\n");
		printf("Sent: %s\n", sent);
		printf("Received: %s\n", recv);
		return -1;
	}
	return 0;
}
