#!/bin/sh
echo "Content-type: text/html"
echo ""

TTT_CHAR_DEV=/dev/ttt_char_dev
CHAR_DEV=/dev/ttyRPMSG0

current=$(cat /sys/class/remoteproc/remoteproc0/firmware)

# Function to extract the 'msg' parameter from a query string or POST data
get_msg_param() {
  local data="$1"
  local data_val

  # Look for the msg parameter in the data
  for pair in $(echo "$data" | tr '&' '\n'); do
    key=$(echo "$pair" | cut -d '=' -f 1)
    value=$(echo "$pair" | cut -d '=' -f 2 | sed 's/%20/ /g')  # Decoding space character
    if [ "$key" == "msg" ]; then
      data_val="$value"
      break
    fi
  done

  echo "$data_val"
}

msg=$(get_msg_param "$QUERY_STRING")

if [ -z "$msg" ]; then
  echo "<p>Error: No 'msg' parameter provided.</p>"
  exit 1
fi

SEND_CMD="echo $msg > $CHAR_DEV"
if [ $current ==  "tictactoe.elf" ]
then
	CHAR_DEV=$TTT_CHAR_DEV
	SEND_CMD="/root/apps/tictactoe_proto_encoder last_msg.txt $msg && cat last_msg.txt > $CHAR_DEV"
fi

if [ "$current" != "tictactoe.elf" ] && [ "$current" != "openamp_tty.elf" ]; then
	echo ""
else
	# Check if file exists
	if [ -e "$CHAR_DEV" ]; then
		eval $SEND_CMD
	fi
fi
