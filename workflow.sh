#!/bin/bash

# Ensure the log_output directory exists
rm -rf log_output
mkdir -p log_output

# Define an array of CMake commands
commands=(
  "cmake --workflow --preset=gcc-12-release"
  "cmake --workflow --preset=gcc-13-release"
  "cmake --workflow --preset=clang-17-libc++release"
  "cmake --workflow --preset=clang-17-release"
)

# Function to sanitize command strings into valid filenames
sanitize_filename() {
    echo "$1" | sed 's/[^a-zA-Z0-9_-]/_/g'
}

# Export the function so it can be used by parallel
export -f sanitize_filename

# Run commands in parallel, each outputting to its own log file
parallel --jobs 0 'command={}; sanitized_filename=$(sanitize_filename "{}"); echo "$command" > log_output/"$sanitized_filename".log; $command &> log_output/"$sanitized_filename".log' ::: "${commands[@]}"

# After all commands have completed, print the outputs sequentially
#for log_file in log_output/*.log; do
#  echo "Output from ${log_file}:"
#  cat "${log_file}"
#  echo "----------------------------------------"
#done

# After all commands have completed, filter and print the outputs for tests passed/failed
for log_file in log_output/*.log; do
  echo "Test results from ${log_file}:"
  grep -E 'tests passed, [0-9]+ tests failed out of [0-9]+' "${log_file}"
  echo "----------------------------------------"
done
