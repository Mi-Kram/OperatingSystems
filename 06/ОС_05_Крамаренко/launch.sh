#!/bin/bash

gcc sender.c -o sender.exe
gcc receiver.c -o receiver.exe

if [ -f ./sender.exe ] && [ -f ./receiver.exe ]; then
  echo "  Usage: ./sender.exe"
  echo "  Usage: ./receiver.exe"
fi

