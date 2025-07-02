#!/bin/bash
# Function to find the maximum number from a list.

# Function to find the maximum number.
find_max() {
  # Default the largest value is the first value.
  max=$1

  # foreach: iterate over all arguments.
  for num in "$@"; do
    # if $num is greater then $max, update maximum.
    if [ "$num" -gt "$max" ]; then
      max=$num
    fi
  done

  echo "The maximum number is: $max"
}

# Function for entering a list of numbers.
get_numbers() {
  # Ask for the numbers and enter them.
  echo "Enter numbers separated by spaces:"
  read -a numbers

  # Find the largest number. ${numbers[@]} - передача массива чисел.
  find_max "${numbers[@]}"
}

# Call function.
get_numbers
