#!/bin/sh
echo "Content-type: text/html"
echo ""

LED_PIN=354
LED_GPIO=/sys/class/gpio/gpio${LED_PIN}

echo "<h3>LED</h3>"
echo "<pre>"
if test -d ${LED_GPIO}; then
	VALUE=$(cat ${LED_GPIO}/value)
	if [ $VALUE == 1 ]; then
		echo "ON"
	else
		echo "OFF"
	fi
else
	echo "OFF"
fi
echo "</pre>"


echo "<h3>UPTIME</h3>"
echo "<pre>"
uptime
echo "</pre>"
echo "<h3>FILESYSTEM</h3>"
echo "<pre>"
df -h
echo "</pre>"
echo "<h3>RAM</h3>"
echo "<pre>"
free -h
echo "</pre>"
echo "<h3>TOP</h3>"
echo "<pre>"
top -bn1 | head -n 10
echo "</pre>"


