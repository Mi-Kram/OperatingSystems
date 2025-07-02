#!/bin/bash
rm program
clear
gcc program.c -o program

if [ -f ./program ]; then
  echo "program is compiled successful"
  echo "Usage: ./program <amount>"
  echo "         <amount> - amount of things to stole, default = 10."
fi




