#!/bin/sh
echo "Content-type: text/html"
echo ""

KWS_CHAR_DEV=/dev/kws_char_dev
TTT_CHAR_DEV=/dev/ttt_char_dev
CHAR_DEV=tty.txt

READ_PROGRAM="tail -n 3"

current=$(cat /sys/class/remoteproc/remoteproc0/firmware)

if [ $current ==  "kws.elf" ]
then
	CHAR_DEV=$KWS_CHAR_DEV
	READ_PROGRAM=/root/apps/kws_proto_decoder
elif [ $current ==  "tictactoe.elf" ]
then
	CHAR_DEV=$TTT_CHAR_DEV
	READ_PROGRAM=/root/apps/tictactoe_proto_decoder
fi

echo "<h3>Status</h3>"
echo "<pre>State: $(cat /sys/class/remoteproc/remoteproc0/state)</pre>"
echo "<pre>Firmware: <pre id="firmware">$current</pre></pre>"

if [ "$current" != "kws.elf" ] && [ "$current" != "tictactoe.elf" ] && [ "$current" != "openamp_tty.elf" ]; then
	echo ""
else
	# Check if file exists
	echo "<h3>Last Message</h3>"
	if [ -e "$CHAR_DEV" ]; then
		echo "<pre>"
		cat $CHAR_DEV > last_msg.txt
		$READ_PROGRAM last_msg.txt
		echo "</pre>"
	else
		echo "<pre>No $CHAR_DEV</pre>"
	fi
	fi
