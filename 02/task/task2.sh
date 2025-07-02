#!/bin/bash
# Program for summing numbers until 0 is entered.

# Function for summing numbers.
sum_numbers() {
  # Variable: the total sum, default 0.
  total=0
  while true; do
    # Ask for a number and read it in $num.
    echo "Enter a number (0 to stop):"
    read num

    # Chech if $ num is zero.
    if [ "$num" -eq 0 ]; then
      break
    fi

    # Else increase the total sum.
    total=$((total + num))
  done
  
  echo "Total sum: $total"
}

# Call function.
sum_numbers
