#!/bin/bash

rm server client
clear

gcc server.c -o server
gcc client.c -o client

if [ -f ./server.exe ] && [ -f ./client.exe ]; then
  echo "Usage:  ./server <address> <port>"
  echo "        ./client <port>"
fi


