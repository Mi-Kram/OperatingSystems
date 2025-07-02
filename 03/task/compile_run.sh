#!/bin/bash

gcc ./main.cpp -o main.exe

echo -n "Enter fibonacci and factorial argument: "
read arg

./main.exe $arg
