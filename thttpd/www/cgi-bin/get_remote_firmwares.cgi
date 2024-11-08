#!/bin/sh
echo "Content-type: text/html"
echo ""

current=$(cat /sys/class/remoteproc/remoteproc0/firmware)
selected=""
for file in /lib/firmware/*.elf; do
	if [ $current ==  $(basename $file) ]; then
		selected="true"
		echo "<option value=\"$(basename $file)\" selected>$(basename $file)</option>"
	else
		echo "<option value=\"$(basename $file)\">$(basename $file)</option>"
	fi
done

if [ $selected == "" ]; then
	echo "<option value=\"none\" selected>None</option>"
fi

