#!/usr/bin/env bash

# Configuration script for ForgeEngine
# This script installs required dependencies, sets default compilers
# and generates the build system.

set -e

######################################################################
# Dependency installation
######################################################################

# Use sudo when not running as root
SUDO=""
if [ "$(id -u)" -ne 0 ]; then
    SUDO="sudo"
fi

install_deps() {
    if command -v apt-get >/dev/null 2>&1; then
        $SUDO apt-get update
        $SUDO apt-get install -y \
            clang build-essential cmake ninja-build git \
            libgl1-mesa-dev libx11-dev libxrandr-dev libxinerama-dev \
            libxcursor-dev libxi-dev libxkbcommon-dev libwayland-dev xorg-dev
    elif command -v dnf >/dev/null 2>&1; then
        $SUDO dnf install -y \
            clang gcc-c++ cmake ninja-build git \
            mesa-libGL-devel libX11-devel libXrandr-devel \
            libXinerama-devel libXcursor-devel libXi-devel \
            libxkbcommon-devel wayland-devel
    else
        echo "Unsupported package manager. Please install dependencies manually." >&2
    fi
}

install_deps

######################################################################
# Build configuration
######################################################################

# Default compilers (can be overridden via environment variables)
export CC="${CC:-clang}"
export CXX="${CXX:-clang++}"

# Build directory
BUILD_DIR="${1:-build}"

# Use Ninja if available, otherwise default to Unix Makefiles
if command -v ninja >/dev/null 2>&1; then
    GENERATOR="Ninja"
else
    GENERATOR="Unix Makefiles"
fi

mkdir -p "$BUILD_DIR"

cmake -S . -B "$BUILD_DIR" -G "$GENERATOR" -DCMAKE_BUILD_TYPE=Release

cmake --build "$BUILD_DIR" -j "$(nproc)"
