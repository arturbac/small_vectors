#!/bin/bash

# Define the search and replace strings
search_string="small_vectors::inline v3_0"
replace_string="namespace small_vectors::inline v3_1"

# Loop over files with specified extensions and perform the replacement
find . -type f \( -name "*.h" -o -name "*.hpp" -o -name "*.cc" -o -name "*.cpp" \) -exec sed -i "s/$search_string/$replace_string/g" {} +

search_string="small_vectors::v3_0"
replace_string="namespace small_vectors::v3_1"

echo "Replacement complete."
