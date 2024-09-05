#!/bin/bash

NMsgs=10000

# Run the tests with no workload
./speed_test -n 10000 -s 4 
mv histogram.txt "Payload_4-NMsg_${NMsgs}.txt"

./speed_test -n 10000 -s 64
mv histogram.txt "Payload_64-NMsg_${NMsgs}.txt"

./speed_test -n 10000 -s 496
mv histogram.txt "Payload_496-NMsg_${NMsgs}.txt"

# Run tests with workload
stress-ng --cpu-method=all -c 4 &

# get the pid of the process
pid=$!

./speed_test -n 10000 -s 4 
mv histogram.txt "Payload_4-NMsg_${NMsgs}_stressed.txt"

./speed_test -n 10000 -s 64
mv histogram.txt "Payload_64-NMsg_${NMsgs}_stressed.txt"

./speed_test -n 10000 -s 496
mv histogram.txt "Payload_496-NMsg_${NMsgs}_stressed.txt"

kill $pid

# Copy results to host
scp Payload_* marcos@192.168.42.27:/home/marcos/facu/tesis/dev/apps/speed_test/histograms/data/bsize512/
