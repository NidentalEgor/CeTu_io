#!/bin/bash

# Set the default configuration to Debug if not provided
CONFIGURATION=${1:-Debug}

# Create the build directory if it does not exist
mkdir -p build
cd build

# Create a subdirectory for the specified configuration
mkdir -p $CONFIGURATION
cd $CONFIGURATION

# Run CMake with the specified configuration
cmake ../.. -DCMAKE_BUILD_TYPE=$CONFIGURATION

# Build the project with the given configuration
cmake --build . --config $CONFIGURATION

# Print a message indicating the build is complete
echo "Build completed with configuration: $CONFIGURATION"
