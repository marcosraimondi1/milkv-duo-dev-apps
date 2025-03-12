# TIC-TAC-TOE
This application is meant for running a simple game of tic-tac-toe with `Mini Max Algorithm`.

## Description
On `milkV Duo` main core you can run the python application (or C application). It will print a tictactoe game on the console, waiting for user input.
The user can play against the remote core (check tictactoe [Zephyr application](https://github.com/marcosraimondi1/milkv-zephyros/tree/main/openamp_tf)). 
It uses protocol buffers to serialize the board and send it to the remote core via RMPMsg protocol.
