#!/bin/bash

# Ensure log directory exists
LOG_DIR=~/runnrs_logs/60s/shapley_soft_hard_weight_1_bumping_50
mkdir -p "$LOG_DIR"

# Check if folder path is given as an argument
if [ -z "$1" ]; then
  echo "Usage: $0 <folder_path>"
  exit 1
fi

# Get the folder path
FOLDER_PATH="$1"

# Iterate over all files in the folder
for file_path in "$FOLDER_PATH"/*; do
  if [ -f "$file_path" ]; then
    # Extract the file name for log file naming
    file_name=$(basename "$file_path")
    log_file="$LOG_DIR/${file_name}.log"

    echo "Run $file_name "

    # Run the program with a timeout of 300 seconds and redirect output to the log file
    timeout 60 ./tt-open-wbo-inc-IntelSATSolver "$file_path" > "$log_file" 2>&1

    # Check if the last command timed out
    if [ $? -eq 124 ]; then
      echo "Timeout: Processing $file_name"
      echo "Timeout: Processing $file_name took too long and was terminated." >> "$log_file"
    fi
  fi
done
