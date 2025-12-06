#!/bin/bash

# Navigate to the project root
cd "$(dirname "$0")"/..

# Remove the existing build directory
rm -rf build

# Create fresh build directory and configure
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_OSX_SYSROOT="$(xcrun --show-sdk-path)"
make -j
