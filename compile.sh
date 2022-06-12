#!/bin/bash
gcc -std=c99 -g -DDEBUG=0 ./server.c -o server
gcc -std=c99 -g -DDEBUG=0 ./client.c -o client