# Netlink Communication Unicast Example

This application shows an example of communication between Linux kernel space and user space.

## Build

Compile kernel module and user space program.

```
make
```

Load kernel module:

```
insmod ./netlink_test.ko
```

Also check kernel log `dmesg` for module debug output.

```
./nl_recv "Hello you!"
Hello you!
```

Unload kernel module:
```
rmmod netlink_test.ko
```
