#!/bin/bash

# Function to check if input is a number 
is_number() {
  echo "$1" | grep -qE '^[0-9]+$'
}

# Function to check if hours are in the correct range (0-23)
# $1: number of hours.
check_hours_range() {
  if [ "$1" -lt 0 ] || [ "$1" -gt 23 ]; then
    return 1  # Invalid range
  else
    return 0  # Valid range
  fi
}

# Function to read the number of hours
read_hours() {
  while true; do
    # Ask for an hours and read it.
    echo "Enter the number of hours (0-23): "
    read input
  
    # Check if input is a valid number
    if ! is_number "$input"; then
      echo "Incorrect input! Try again."
      continue
    fi
  
    # Check if the hours are in a valid range
    if ! check_hours_range "$input"; then
      echo "Incorrect number of hours! Try again."
      continue
    fi

    # Store valid hours and break the loop
    hours="$input"
    break
  done
}

# Function to print a greeting message based on hours
greet() {
  if [ "$1" -ge 22 ] || [ "$1" -lt 5 ]; then  # 22:00 - 05:00
    echo "Good night!"
  elif [ "$1" -lt 11 ]; then  # 05:00 - 11:00
    echo "Good morning!"
  elif [ "$1" -lt 18 ]; then  # 11:00 - 18:00
    echo "Good afternoon!"
  else                        # 18:00 - 22:00
    echo "Good evening!"
  fi
}

# Main function
main() {
  read_hours  # Get valid hours
  greet "$hours"  # Print greeting based on the hours
}

# Run main function
main
