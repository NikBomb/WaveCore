#!/usr/bin/env bash
set -euo pipefail

PRESET="${1:-gcc14-debug}"

echo "Configuring with preset: ${PRESET}"
cmake --preset "${PRESET}"

echo "Building with preset: ${PRESET}"
cmake --build --preset "${PRESET}"

echo "Running tests with preset: ${PRESET}"
ctest --preset "${PRESET}"

echo "Done."