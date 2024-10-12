#!/bin/sh
echo "Content-type: text/html"
echo ""

CHAR_DEV=/dev/rpmsg_char_dev

echo "<h3>Status</h3>"
echo "<pre>State: $(cat /sys/class/remoteproc/remoteproc0/state)</pre>"
echo "<pre>Firmware: $(cat /sys/class/remoteproc/remoteproc0/firmware)</pre>"

# check if /dev/rpmsg_char_dev exists
echo "<h3>Last Message</h3>"
if [ -e $CHAR_DEV ]; then
    echo "<pre>"
    /usr/bin/kws_protobuf_decoder $CHAR_DEV
    echo "</pre>"
else
    echo "<pre>No $CHAR_DEV</pre>"
fi
