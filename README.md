# Milkv Duo Application Development Setup
Userspace application that run on embedded linux for milkv duo boards.

## Build
1. Clone the repo:
```sh
git clone https://github.com/marcosraimondi1/milkv-duo-dev-apps.git apps
cd apps
```
2. Source sdk config (it may take some time the first time is run as it downloads the toolchain):
```sh
source envsetup.sh
```
4. Build apps:
```sh
cd helloworld
make
```
5. Load app to duo (connect duo with usb, default ssh password is milkv):
```sh
scp helloworld root@192.168.42.1:/root/
```
6. SSH into duo's terminal and run the app:
```sh
ssh root@192.168.42.1
./helloworld
```

## Note
Read each app README.md for additional configuration.
