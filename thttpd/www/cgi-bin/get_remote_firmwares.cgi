#!/bin/sh
echo "Content-type: text/html"
echo ""

for file in /lib/firmware/*.elf; do
	# return filename
	echo "<option value=\"$(basename $file)\">$(basename $file)</option>"
done

echo "<option value=\"stop\">stop</option>"
