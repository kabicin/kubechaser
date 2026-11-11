#!/usr/bin/env bash

set -euo pipefail

build_app=OFF
build_dir=build-core

if [[ "${1:-}" == "--full" ]]; then
    build_app=ON
    build_dir=build
elif [[ $# -ne 0 ]]; then
    echo "Usage: $0 [--full]" >&2
    exit 2
fi

cmake -S . -B "${build_dir}" \
    -DKUBECHASER_BUILD_APP="${build_app}" \
    -DBUILD_TESTING=ON
cmake --build "${build_dir}" --parallel
ctest --test-dir "${build_dir}" --output-on-failure
