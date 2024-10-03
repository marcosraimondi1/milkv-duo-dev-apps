#!/bin/sh
echo "Content-type: text/html"
echo ""

state=$(cat /sys/class/remoteproc/remoteproc0/state)
if [ $state == "running" ]; then
	echo stop > /sys/class/remoteproc/remoteproc0/state
	echo "Success!"
else
	echo "No remote firmware is running"
fi
