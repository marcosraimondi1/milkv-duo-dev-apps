#!/bin/sh

echo "install thttpd..."
cp -a bin/* /usr/bin
cp -a etc/* /etc
cp -r -a www/* /var/www
cp S85thttpd /etc/init.d
echo "done!"
