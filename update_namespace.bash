#!/bin/bash

# Define the search and replace strings
search_string="small_vectors::inline v3_2"
replace_string="small_vectors::inline v3_3"

# Loop over files with specified extensions and perform the replacement
find . -type f \( -name "*.h" -o -name "*.hpp" -o -name "*.cc" -o -name "*.cpp" \) -exec sed -i "s/$search_string/$replace_string/g" {} +

search_string="small_vectors::v3_0"
replace_string="small_vectors::v3_2"

echo "Replacement complete."
