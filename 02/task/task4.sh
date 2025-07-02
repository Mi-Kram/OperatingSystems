#!/bin/bash
# Function for input checking with loop.

# Function to check if input is a number.
# $1: input string.
# returns: 0 if it is a number, otherwise 1.
is_number() {
  if echo "$1" | grep -qE '^[0-9]+$'; then
    return 0
  else
    return 1
  fi
}

# Function to get the correct number.
get_number() {
  while true; do
    # Ask for an hours and read it.
    echo "Enter a number:"
    read number

    # Check if input is a number.
    if is_number "$number"; then
      echo "You entered: $number"
      break
    else
      echo "Invalid input, please enter a valid number."
    fi
  done
}

# Call function.
get_number
