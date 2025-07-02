#!/bin/bash

rm writer reader

gcc writer.c common.c -o writer
gcc reader.c common.c -o reader

if [ -f ./writer ] && [ -f ./reader ]; then
  echo "Usage:  ./writer"
  echo "        ./reader"
fi


