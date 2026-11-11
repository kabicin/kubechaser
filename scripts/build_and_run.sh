#!/usr/bin/env bash

set -euo pipefail

# build
cmake -S . -B build
cmake --build build --parallel

# run
./build/kubechaser
