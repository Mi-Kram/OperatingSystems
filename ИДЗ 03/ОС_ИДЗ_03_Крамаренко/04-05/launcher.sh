#!/bin/bash

rm server ivanov petrov necheporuk
clear

gcc -o server server.c
gcc -o ivanov ivanov.c
gcc -o petrov petrov.c
gcc -o necheporuk necheporuk.c

if [ -f ./server ] && [ -f ./ivanov ] && [ -f ./petrov ] && [ -f ./necheporuk ]; then
  echo "programs are compiled successful"
  echo "Usage:  ./server <address> <port>"
  echo "        ./ivanov <address> <port> <amount, default 10>"
  echo "        ./petrov <address> <port>"
  echo "        ./necheporuk <address> <port>"
fi

