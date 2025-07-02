#!/bin/bash

rm server sender receiver
clear

gcc -o server server.c
gcc -o sender sender.c
gcc -o receiver receiver.c

if [ -f ./server ] && [ -f ./sender ] && [ -f ./receiver ]; then
  echo "programs are compiled successful"
  echo "Usage:  ./server <ip> <port>"
  echo "        ./sender <ip> <port>"
  echo "        ./receiver <ip> <port>"
fi

