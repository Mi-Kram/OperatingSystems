#!/bin/bash

rm server ivanov petrov necheporuk logger
clear

gcc -o server server.c
gcc -o ivanov ivanov.c
gcc -o petrov petrov.c
gcc -o necheporuk necheporuk.c
gcc -o logger logger.c

if [ -f ./server ] && [ -f ./ivanov ] && [ -f ./petrov ] && [ -f ./necheporuk ] && [ -f ./logger ]; then
  echo "programs are compiled successful"
  echo "Usage:  ./server <address> <port> <logger_address> <logger_port>"
  echo "        ./ivanov <address> <port> <amount, default 10>"
  echo "        ./petrov <address> <port>"
  echo "        ./necheporuk <address> <port>"
  echo "        ./logger <logger_address> <logger_port>"
fi

