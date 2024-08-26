#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

#define RPMSG_DEV "/dev/ttyRPMSG0"
#define LOADING_BAR_LENGTH 50

void print_loading_bar(int progress);

int main(int argc, char **argv) {
  if (argc < 3) {
    fprintf(stderr, "Usage: %s <message_size> <num_messages>\n", argv[0]);
    return -1;
  }

  int MESSAGE_SIZE = atoi(argv[1]);
  int NUM_MESSAGES = atoi(argv[2]);

  int fd = open(RPMSG_DEV, O_RDWR);
  if (fd < 0) {
    perror("Failed to open RPMsg device");
    return -1;
  }

  char msg[MESSAGE_SIZE];
  for (int i = 0; i < MESSAGE_SIZE - 2; i++) {
    msg[i] = 'a';
  }
  msg[MESSAGE_SIZE - 2] = '\n'; // Newline character
  msg[MESSAGE_SIZE - 1] = '\0'; // Null-terminate the string

  struct timespec start, end;
  long total_time_ns = 0;

  for (int i = 0; i < NUM_MESSAGES; i++) {
    // Measure the time taken to send and receive a single message
    clock_gettime(CLOCK_MONOTONIC, &start);

    int len = write(fd, msg, MESSAGE_SIZE);
    if (len < 0) {
      perror("Failed to send message");
      fprintf(stderr, "message number %d", i);
      close(fd);
      return -1;
    }

    char buffer[MESSAGE_SIZE];
    len = read(fd, msg, MESSAGE_SIZE);
    if (len < 0) {
      perror("Failed to receive message");
      close(fd);
      return -1;
    }

    clock_gettime(CLOCK_MONOTONIC, &end);
    total_time_ns += (end.tv_sec - start.tv_sec) * 1000000000L +
                     (end.tv_nsec - start.tv_nsec);

    print_loading_bar((i + 1) * LOADING_BAR_LENGTH / NUM_MESSAGES);
  }

  double average_time_s = total_time_ns / (double)NUM_MESSAGES / 1e9;
  double throughput = NUM_MESSAGES * MESSAGE_SIZE / total_time_ns / 1e9 /
                      1024.0 / 1024.0; // MB/s

  printf("\nSent %d messages, each of size %d bytes\n", NUM_MESSAGES,
         MESSAGE_SIZE);
  printf("Total time: %.2f seconds\n", total_time_ns / 1e9);
  printf("Average time per message: %.6f seconds\n", average_time_s);
  printf("Throughput: %.2f MB/s\n", throughput);

  close(fd);
  return 0;
}

void print_loading_bar(int progress) {
  printf(
      "\r["); // Return to the start of the line and print the opening bracket

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
