# KWS Application

This application is meant for running on MilkV Duo main core and interact with a remote core application through RPMsg [Zephyr KWS Application](https://github.com/marcosraimondi1/milkv-zephyros/tree/main/kws).

## Description
The remote application runs inferences on captured audio using the ADC. 
Every inference is then serialized with protocol buffers and sent via RPMsg to the main core. 
This application connects via netlink to the RPMsg kernel module and prints the result on the console.

A python and C version of the program are offered.
