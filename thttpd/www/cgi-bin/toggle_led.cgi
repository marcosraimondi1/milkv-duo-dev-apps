#!/bin/sh
echo "Content-type: text/html"
echo ""

LED_PIN=354
LED_GPIO=/sys/class/gpio/gpio${LED_PIN}

if ! test -d ${LED_GPIO}; then
    echo ${LED_PIN} > /sys/class/gpio/export
fi

VALUE=$(cat ${LED_GPIO}/value)

echo out > ${LED_GPIO}/direction

if [ $VALUE == 1 ]; then
	echo 0 > ${LED_GPIO}/value
	echo "LED turned OFF"
else
	echo 1 > ${LED_GPIO}/value
	echo "LED turned ON"
fi


