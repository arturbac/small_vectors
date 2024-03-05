#!/bin/bash

# Define directories to skip
SKIP_DIRS=("build" "cmake")

# Function to check if directory should be skipped
should_skip() {
  for dir in "${SKIP_DIRS[@]}"; do
    if [[ "$1" == *"/$dir/"* ]]; then
      return 0 # 0 means true in bash, so skip
    fi
  done
  return 1 # 1 means false in bash, so don't skip
}

# Main loop to find and format files
find . -type f \( -iname \*.h -o -iname \*.hpp -o -iname \*.cc -o -iname \*.c -o -iname \*.cpp \) | while read -r file; do
  if should_skip "$file"; then
#    echo "Skipping $file"
    continue
  fi
  
  # Create a temporary file to store the original content
  temp_file=$(mktemp)
  cp "$file" "$temp_file"
  
  # Format the file
  clang-format -i "$file"
  
  # Check if the file was changed by comparing it to the temporary file
  if ! cmp -s "$file" "$temp_file"; then
    echo "Formatted $file"
#  else
#    echo "$file unchanged."
  fi
  
  # Clean up the temporary file
  rm "$temp_file"
done

echo "Formatting complete."
