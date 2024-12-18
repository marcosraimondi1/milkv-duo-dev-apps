#!/bin/bash
echo "Content-type: text/html"
echo ""

# Function to extract the 'firmware' parameter from a query string or POST data
get_firmware_param() {
  local data="$1"
  local firmware_value

  # Look for the firmware parameter in the data
  for pair in $(echo "$data" | tr '&' '\n'); do
    key=$(echo "$pair" | cut -d '=' -f 1)
    value=$(echo "$pair" | cut -d '=' -f 2 | sed 's/%20/ /g')  # Decoding space character
    if [ "$key" == "firmware" ]; then
      firmware_value="$value"
      break
    fi
  done

  echo "$firmware_value"
}

/sbin/rmmod kws_mod > /dev/null 2>&1
/sbin/rmmod tictactoe_mod > /dev/null 2>&1

firmware=$(get_firmware_param "$QUERY_STRING")

if [ -z "$firmware" ]; then
  echo "<p>Error: No 'firmware' parameter provided.</p>"
else
  dmesg -C
 
  state=$(cat /sys/class/remoteproc/remoteproc0/state)
  if [ $state == "running" ]; then
    echo -n stop > /sys/class/remoteproc/remoteproc0/state
  fi

  echo -n "$firmware" > /sys/class/remoteproc/remoteproc0/firmware
  echo -n start > /sys/class/remoteproc/remoteproc0/state
  dmesg

  sleep 1

  echo "" > last_msg.txt

  if [ $firmware == "openamp_tty.elf" ]
  then
	nohup cat /dev/ttyRPMSG0 &>tty.txt &
  fi

  if [ $firmware == "tictactoe.elf" ]
  then
        /sbin/insmod /root/modules/tictactoe_mod.ko
  fi

  if [ $firmware == "kws.elf" ]
  then
        /sbin/insmod /root/modules/kws_mod.ko
  fi
fi
