#!/bin/bash

# build
cmake -S . -B build
cmake --build build

# make and run
cd build
make
./kubechaser
cd ..