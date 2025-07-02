#!/bin/bash

rm server.exe client.exe

gcc server.c -o server.exe
gcc client.c -o client.exe

if [ -f ./server.exe ] && [ -f ./client.exe ]; then
  echo "Usage:  ./server.exe"
  echo "        ./client.exe <server pid>"
fi


