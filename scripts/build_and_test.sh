#!/usr/bin/env bash
set -euo pipefail

BUILD_TYPE="${1:-Debug}"
BUILD_DIR="build/${BUILD_TYPE,,}"

echo "Configuring WaveCore..."
cmake -S . -B "${BUILD_DIR}" \
  -DCMAKE_BUILD_TYPE="${BUILD_TYPE}" \
  -DWAVECORE_ENABLE_TESTS=ON

echo "Building WaveCore..."
cmake --build "${BUILD_DIR}" --parallel

echo "Running tests..."
ctest --test-dir "${BUILD_DIR}" --output-on-failure

echo "Done."
