#!/bin/sh

echo "install thttpd..."
cp -a bin/* /usr/bin
cp -a etc/* /etc
cp -r -a www/* /var/www
cp S85thttpd /etc/init.d

# update permissions
chmod 777 /usr/bin/thttpd
chmod 644 /var/www/index.html
chmod 644 /var/www/js/*
chmod 644 /var/www/css/*
chmod 644 /var/www/cgi-bin/*
chmod +x /var/www/cgi-bin/*

echo "done!"
