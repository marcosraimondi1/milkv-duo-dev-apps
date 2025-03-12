# THTTPD program

Compiled binary for `MilkV Duo` for running a thttpd web server. It includes files for running a web application to interact with the milkV board including managing the remote processor using **Remoteproc** and **RPMsg**.  

## Build

Copy directory to milkv embedded linux and run `install.sh` script to copy the files to the correct location.

The webserver is accesible through the MilkV RNDIS interface, default ip is `192.168.42.1`. (http://192.168.42.1).
