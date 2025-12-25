#!/bin/bash

set -e

git pull

mkdir -p build
cd build

cmake ..

cmake --build .

echo "Build successful!"
