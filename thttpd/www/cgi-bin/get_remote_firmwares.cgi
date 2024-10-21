#!/bin/sh
echo "Content-type: text/html"
echo ""

current=$(cat /sys/class/remoteproc/remoteproc0/firmware)

for file in /lib/firmware/*.elf; do
        if [ $current ==  $(basename $file) ]; then
                echo "<option value=\"$(basename $file)\" selected>$(basename $file)</option>"
        else
                echo "<option value=\"$(basename $file)\">$(basename $file)</option>"
        fi
done
