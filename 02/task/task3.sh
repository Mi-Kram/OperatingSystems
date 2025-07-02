#!/bin/bash
# Program for determining the range of time of day.

# Function for determining the range of time of day.
# $1: number of hours.
greet() {
  if [ "$1" -ge 0 ] && [ "$1" -lt 6 ]; then
    echo "Good night!"
  elif [ "$1" -ge 6 ] && [ "$1" -lt 12 ]; then
    echo "Good morning!"
  elif [ "$1" -ge 12 ] && [ "$1" -lt 18 ]; then
    echo "Good afternoon!"
  else
    echo "Good evening!"
  fi
}

# Main function.
get_time() {
  # Ask for an hours and read it.
  echo "Enter the current hour (0-23):"
  read hour

  # Print greeting.
  greet "$hour"
}

# Call function.
get_time
