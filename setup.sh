#!/usr/bin/env bash

# Configuration script for ForgeEngine
# This script sets default compilers and generates the build system.

set -e

# Default compilers (can be overridden via environment variables)
export CC=${CC:-/usr/bin/clang}
export CXX=${CXX:-/usr/bin/clang++}

# Build directory
BUILD_DIR=${1:-build}

# Use Ninja if available, otherwise default to Unix Makefiles
if command -v ninja >/dev/null 2>&1; then
    GENERATOR="Ninja"
else
    GENERATOR="Unix Makefiles"
fi

mkdir -p "$BUILD_DIR"

cmake -S . -B "$BUILD_DIR" -G "$GENERATOR" -DCMAKE_BUILD_TYPE=Release

cmake --build "$BUILD_DIR" -j $(nproc)