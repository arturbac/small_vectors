cmake_minimum_required(VERSION 3.14) # Require at least CMake 3.14 for string(PREPEND ...)

# Read the version header file
file(READ include/small_vectors/version.h version_file_contents)

# Extract the version number
string(REGEX MATCH "SMALL_VECTORS_VERSION \"([0-9]+\\.[0-9]+\\.[0-9]+)\"" _ ${version_file_contents})
set(small_vectors_version ${CMAKE_MATCH_1})

