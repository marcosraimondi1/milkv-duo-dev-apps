/*
 * rpmsg_char_simple.c
 *
 * Simple Example application using rpmsg-char library
 *
 * Copyright (c) 2020 Texas Instruments Incorporated - https://www.ti.com
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * *  Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * *  Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * *  Neither the name of Texas Instruments Incorporated nor the names of
 *    its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 *  A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 *  OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 *  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 *  LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 *  DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 *  THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 *  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 *  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */

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

#define NUM_ITERATIONS 100

/*
 * This test can measure round-trip latencies up to 20 ms
 * Latencies measured in microseconds (us)
 */
#define LATENCY_RANGE 20000

/* message size options */
#define RPMSG_BUFFER_SIZE 4096
enum test_msg_size {
	MSG_SIZE_MIN = 5,
	MSG_SIZE_NORMAL = 65,
	MSG_SIZE_MAX = RPMSG_BUFFER_SIZE - 16 + 1,
};

int rpmsg_init(void);
int rpmsg_exit(int fd);
int send_msg(int fd, char *msg, int len);
int recv_msg(int fd, int len, char *reply_msg, int *reply_len);
int rpmsg_char_ping(char *dev_name, int num_msgs, enum test_msg_size msg_size);

void usage()
{
	printf("Usage: rpmsg_char_simple [-n <num_msgs>] [-d <rpmsg_dev_name>] [-s <msg_size>] \n");
	printf("\t\tDefaults: num_msgs: %d rpmsg_dev_name: /dev/ttyRPMSG0 msg_size: normal (min, "
	       "max)\n",
	       NUM_ITERATIONS);
}

int main(int argc, char *argv[])
{
	int ret, status, c;
	int num_msgs = NUM_ITERATIONS;
	char default_dev_name[] = "/dev/ttyRPMSG0";
	char *dev_name = default_dev_name;
	enum test_msg_size msg_size = MSG_SIZE_NORMAL;

	while (1) {
		c = getopt(argc, argv, "n:d:s:");
		if (c == -1) {
			break;
		}

		switch (c) {
		case 's':
			if (strcmp(optarg, "min") == 0) {
				msg_size = MSG_SIZE_MIN;
			} else if (strcmp(optarg, "max") == 0) {
				msg_size = MSG_SIZE_MAX;
			} else {
				msg_size = MSG_SIZE_NORMAL;
			}
			break;
		case 'n':
			num_msgs = atoi(optarg);
			break;
		case 'd':
			dev_name = optarg;
			break;
		default:
			usage();
			exit(0);
		}
	}

	status = rpmsg_char_ping(dev_name, num_msgs, msg_size);

	if (status < 0) {
		printf("TEST STATUS: FAILED\n");
	} else {
		printf("TEST STATUS: PASSED\n");
	}

	return 0;
}

/* single thread communicating with a single endpoint */
int rpmsg_char_ping(char *dev_name, int num_msgs, enum test_msg_size msg_size)
{
	int ret = 0;
	int i = 0;
	int packet_len;
	int packet_recv_len;
	int flags = 0;
	/*
	 * Each RPMsg packet can have up to 496 bytes of data:
	 * 512 bytes total - 16 byte header = 496
	 */
	char packet_send_buf[MSG_SIZE_MAX] = {0};
	char packet_recv_buf[MSG_SIZE_MAX] = {0};

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

	int fd = rpmsg_init();

	if (fd < 0) {
		printf("rpmsg_init failed\n");
		return -1;
	}

	memset(packet_send_buf, 0, sizeof(packet_send_buf));

	snprintf(packet_send_buf, msg_size,
		 "01234567890123456789012345678901234567890123456789012345678901234567890123"
		 "45678901234567890123456789012345678901234567890123456789012345678901234567"
		 "89012345678901234567890123456789012345678901234567890123456789012345678901"
		 "23456789012345678901234567890123456789012345678901234567890123456789012345"
		 "67890123456789012345678901234567890123456789012345678901234567890123456789"
		 "01234567890123456789012345678901234567890123456789012345678901234567890123"
		 "45678901234567890123456789012345678901234567890123456789012345678901234567"
		 "89012345678901234567890123456789012345678901234567890123456789012345678901"
		 "23456789012345678901234567890123456789012345678901234567890123456789012345"
		 "67890123456789012345678901234567890123456789012345678901234567890123456789"
		 "01234567890123456789012345678901234567890123456789012345678901234567890123"
		 "45678901234567890123456789012345678901234567890123456789012345678901234567"
		 "89012345678901234567890123456789012345678901234567890123456789012345678901"
		 "23456789012345678901234567890123456789012345678901234567890123456789012345"
		 "67890123456789012345678901234567890123456789012345678901234567890123456789"
		 "01234567890123456789012345678901234567890123456789012345678901234567890123"
		 "45678901234567890123456789012345678901234567890123456789012345678901234567"
		 "89012345678901234567890123456789012345678901234567890123456789012345678901"
		 "23456789012345678901234567890123456789012345678901234567890123456789012345"
		 "67890123456789012345678901234567890123456789012345678901234567890123456789"
		 "01234567890123456789012345678901234567890123456789012345678901234567890123"
		 "45678901234567890123456789012345678901234567890123456789012345678901234567"
		 "89012345678901234567890123456789012345678901234567890123456789012345678901"
		 "23456789012345678901234567890123456789012345678901234567890123456789012345"
		 "67890123456789012345678901234567890123456789012345678901234567890123456789"
		 "01234567890123456789012345678901234567890123456789012345678901234567890123"
		 "45678901234567890123456789012345678901234567890123456789012345678901234567"
		 "89012345678901234567890123456789012345678901234567890123456789012345678901"
		 "23456789012345678901234567890123456789012345678901234567890123456789012345"
		 "67890123456789012345678901234567890123456789012345678901234567890123456789"
		 "01234567890123456789012345678901234567890123456789012345678901234567890123"
		 "45678901234567890123456789012345678901234567890123456789012345678901234567"
		 "89012345678901234567890123456789012345678901234567890123456789012345678901"
		 "23456789012345678901234567890123456789012345678901234567890123456789012345"
		 "67890123456789012345678901234567890123456789012345678901234567890123456789"
		 "01234567890123456789012345678901234567890123456789012345678901234567890123"
		 "45678901234567890123456789012345678901234567890123456789012345678901234567"
		 "89012345678901234567890123456789012345678901234567890123456789012345678901"
		 "23456789012345678901234567890123456789012345678901234567890123456789012345"
		 "67890123456789012345678901234567890123456789012345678901234567890123456789"
		 "01234567890123456789012345678901234567890123456789012345678901234567890123"
		 "45678901234567890123456789012345678901234567890123456789012345678901234567"
		 "89012345678901234567890123456789012345678901234567890123456789012345678901"
		 "23456789012345678901234567890123456789012345678901234567890123456789012345"
		 "67890123456789012345678901234567890123456789012345678901234567890123456789"
		 "01234567890123456789012345678901234567890123456789012345678901234567890123"
		 "45678901234567890123456789012345678901234567890123456789012345678901234567"
		 "89012345678901234567890123456789012345678901234567890123456789012345678901"
		 "23456789012345678901234567890123456789012345678901234567890123456789012345"
		 "67890123456789012345678901234567890123456789012345678901234567890123456789"
		 "01234567890123456789012345678901234567890123456789012345678901234567890123"
		 "45678901234567890123456789012345678901234567890123456789012345678901234567"
		 "89012345678901234567890123456789012345678901234567890123456789012345678901"
		 "23456789012345678901234567890123456789012345678901234567890123456789012345"
		 "67890123456789012345678901234567890123456789012345678901234567890123456789"
		 "0123456789");
	packet_len = strlen(packet_send_buf);

	/* double-check: is packet_len changing, or fixed at 496? */
	printf("packet_len = %d\n", packet_len);

	/* remove prints to speed up the test execution time */
	// printf("Sending message #%d: %s\n", i, packet_buf);

	printf("Starting test with %d messages of size %d bytes\n", num_msgs, packet_len);

	clock_gettime(CLOCK_MONOTONIC, &ts_start_test);
	for (i = 0; i < num_msgs; i++) {

		clock_gettime(CLOCK_MONOTONIC, &ts_current);
		ret = send_msg(fd, (char *)packet_send_buf, packet_len);
		if (ret < 0) {
			printf("send_msg failed for iteration %d, ret = %d\n", i, ret);
			goto out;
		}

		ret = recv_msg(fd, packet_len, (char *)packet_recv_buf, &packet_recv_len);
		if (ret < 0) {
			printf("recv_msg failed for iteration %d, ret = %d\n", i, ret);
			goto out;
		}
		if (packet_recv_len != packet_len) {
			printf("bytes written does not match received, sent = %d, recv = %d\n",
			       packet_len, packet_recv_len);
			printf("packet_send_buf = %s\n", packet_send_buf);
			printf("packet_recv_buf = %s\n", packet_recv_buf);
			goto out;
		}
		clock_gettime(CLOCK_MONOTONIC, &ts_end);

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

	printf("\nCommunicated %d messages successfully on %s\n\n", num_msgs, dev_name);
	printf("Total execution time for the test: %ld seconds\n",
	       ts_end_test.tv_sec - ts_start_test.tv_sec);
	printf("Average round-trip latency: %f [us]\n", latency_average);
	printf("Worst-case round-trip latency: %d [us]\n", latency_worst_case);
	printf("Histogram data at histogram.txt\n");

out:
	rpmsg_exit(fd);

	return ret;
}

// #define USE_RPMSG_TTY
#ifdef USE_RPMSG_TTY
int rpmsg_init()
{
	/*
	 * Open the remote rpmsg device identified by dev_name and bind the
	 * device to a local end-point used for receiving messages from
	 * remote processor
	 */
	int fd = open("/dev/ttyRPMSG0", O_RDWR);
	if (fd < 0) {
		perror("Can't open rpmsg endpt device\n");
		return -1;
	}
	return fd;
}

int send_msg(int fd, char *msg, int len)
{
	int ret = 0;

	ret = write(fd, msg, len);
	if (ret < 0) {
		perror("Can't write to rpmsg endpt device\n");
		return -1;
	}

	return ret;
}

int recv_msg(int fd, int len, char *reply_msg, int *reply_len)
{
	int ret = 0;

	/* Note: len should be max length of response expected */
	ret = read(fd, reply_msg, len);
	if (ret < 0) {
		perror("Can't read from rpmsg endpt device\n");
		return -1;
	} else {
		*reply_len = ret;
	}

	return 0;
}

int rpmsg_exit(int fd)
{
	close(fd);
	return 0;
}

#else
#include <sys/socket.h>
#include <linux/netlink.h>
#include <errno.h>
#define NETLINK_ID 17
struct sockaddr_nl src_addr, dest_addr;
struct nlmsghdr *nlh_send, *nlh_recv;

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

	// Allocate netlink message header
	nlh_send = (struct nlmsghdr *)malloc(NLMSG_SPACE(MSG_SIZE_MAX));
	nlh_recv = (struct nlmsghdr *)malloc(NLMSG_SPACE(MSG_SIZE_MAX));

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
	*reply_len = ret - NLMSG_HDRLEN;
	memcpy(reply_msg, msg, *reply_len); // this adds latency
	return 0;
}

#endif
