#!/bin/bash

NMsgs=10000

# Run the tests
./speed_test -n 10000 -s 4 
mv histogram.txt "Payload_4-NMsg_${NMsgs}.txt"

./speed_test -n 10000 -s 64
mv histogram.txt "Payload_64-NMsg_${NMsgs}.txt"

./speed_test -n 10000 -s 496
mv histogram.txt "Payload_496-NMsg_${NMsgs}.txt"

# Copy results to host
scp Payload_* marcos@192.168.42.27:/home/marcos/facu/tesis/dev/apps/speed_test/histograms/data/bsize512/

