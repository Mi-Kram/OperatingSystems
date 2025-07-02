#!/bin/bash
# Program to check if a number is even.

# Function to check if a number is even.
# $1: number.
check_even() {
  if [ $(($1 % 2)) -eq 0 ]; then
    echo "$1 is even."
  else
    echo "$1 is odd."
  fi
}

# Function to receive input and check number.
get_input() {
  # Ask for a number and read it.
  echo "Enter a number:"
  read number

  # Check if a number is even.
  check_even "$number"
}

# Call function.
get_input
