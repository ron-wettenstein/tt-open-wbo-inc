#!/bin/bash

# Ensure log directory exists
LOG_DIR=~/runnrs_logs/auto/60s/
mkdir -p "$LOG_DIR"

# Check if folder path is given as an argument
if [ -z "$1" ]; then
  echo "Usage: $0 <folder_path>"
  exit 1
fi

# game theory options, will also be the folder in the logs file
options=("banzhaf_only_hard_bumping_50" "banzhaf_hard_weight_1_bumping_200" "banzhaf_hard_weight_1_bumping_10" "banzhaf_hard_weight_3_bumping_50" "banzhaf_hard_weight_10_bumping_50")

# Get the folder path
FOLDER_PATH="$1"

# Iterate over all files in the folder
for option in "${options[@]}"; do
  echo "!!!!! Running ${option} !!!!!!"
  mkdir "$LOG_DIR/${option}"
  for file_path in "$FOLDER_PATH"/*; do
    if [ -f "$file_path" ]; then
      # Extract the file name for log file naming
      file_name=$(basename "$file_path")
      log_file="$LOG_DIR/${option}/${file_name}.log"

      echo "Run $file_name "

      # Run the program with a timeout of 300 seconds and redirect output to the log file
      timeout 60 ./tt-open-wbo-inc-IntelSATSolver "$file_path" "$option" > "$log_file" 2>&1

      # Check if the last command timed out
      if [ $? -eq 124 ]; then
        echo "Timeout: Processing $file_name"
        echo "Timeout: Processing $file_name took too long and was terminated." >> "$log_file"
      fi
    fi
  done
done
