#!/bin/bash
gcc -std=c99 -g -DDEBUG=1 ./server.c -o server
gcc -std=c99 -g -DDEBUG=1 ./client.c -o client
