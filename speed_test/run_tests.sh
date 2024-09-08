#!/bin/bash

NMsgs=100000
NTESTS=11
ARRAY=(4 8 16 32 64 128 256 496 1024 2032 4080)

# Clear the kernel log
dmesg -C

# start remote processor
echo start > /sys/class/remoteproc/remoteproc0/state

# Load the kernel module
insmod rpmsg_netlink.ko

# Run the tests with no workload
for ((i = 0; i < ${NTESTS}; i++)); do
  payload_size=${ARRAY[i]}
  ./speed_test -n ${NMsgs} -s ${payload_size}
  mv histogram.txt "Payload_${payload_size}-NMsg_${NMsgs}.txt"
done

# Run tests with workload
stress-ng --cpu-method=all -c 4 &

# get the pid of the process
pid=$!

for ((i = 0; i < ${NTESTS}; i++)); do
  payload_size=${ARRAY[i]}
  ./speed_test -n ${NMsgs} -s ${payload_size}
  mv histogram.txt "Payload_${payload_size}-NMsg_${NMsgs}_stressed.txt"
done

# Copy results to host
scp Payload_* marcos@192.168.42.135:/home/marcos/facu/tesis/dev/apps/speed_test/histograms/data/bsize2048/

echo "Tests finished, Ctrl+C to kill stress-ng"
fg
