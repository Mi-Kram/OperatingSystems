#!/bin/bash
rm ivanov petrov necheporuk
clear
gcc ivanov.c -o ivanov
gcc petrov.c -o petrov
gcc necheporuk.c -o necheporuk

if [ -f ./ivanov ] && [ -f ./petrov ] && [ -f ./necheporuk ]; then
  echo "programs are compiled successful"
  echo "Usage: ./ivanov <amount>"
  echo "         <amount> - amount of things to stole, default = 10."
  echo "Usage: ./petrov"
  echo "Usage: ./necheporuk"
fi

