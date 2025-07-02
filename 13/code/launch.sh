#!/bin/bash

rm main
clear

gcc main.c -o main

if [ -f ./main ]; then
  ./main
fi


