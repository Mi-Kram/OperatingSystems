#!/bin/bash
rm program1.exe program2.exe
clear
gcc program1.c -o program1.exe
gcc program2.c -o program2.exe

if [ -f ./program1.exe ] && [ -f ./program2.exe ]; then
  echo "program1.exe and program2.exe are compiled successful"
  echo "Usage program1.exe: ./program1.exe input.txt output.txt fifo1 fifo2"
  echo "Usage program2.exe: ./program2.exe fifo1 fifo2"
fi




