#!/bin/bash

# Path to the version header file
VERSION_FILE="include/small_vectors/version.h"

# Extract the version number from the file
VERSION=$(grep 'SMALL_VECTORS_VERSION' $VERSION_FILE | sed 's/.*"\(.*\)".*/\1/')

# Check if the version was successfully extracted
if [ -z "$VERSION" ]; then
    echo "Error: Could not extract version from $VERSION_FILE"
    exit 1
fi

# Create a git tag with the extracted version, prefixed by 'v'
TAG="v$VERSION"

echo "Creating git tag: $TAG"

# Check if the tag already exists
if git rev-parse "$TAG" >/dev/null 2>&1; then
    echo "Error: Tag $TAG already exists."
    exit 1
else
    git tag -a "$TAG" -m "Version $VERSION"
    echo "Git tag $TAG created successfully."

fi
