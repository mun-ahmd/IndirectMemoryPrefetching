#!/bin/bash
# Build script for LoongArch runtime library
# Usage: ./build_loongarch.sh [GCC_TOOLCHAIN_PATH]

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
GCC_TOOLCHAIN_PATH="${1:-/home/muneeb/repos/IndirectMemoryPrefetching/loongarch-tools}"

if [ ! -d "$GCC_TOOLCHAIN_PATH" ]; then
    echo "Error: GCC_TOOLCHAIN_PATH '$GCC_TOOLCHAIN_PATH' does not exist"
    exit 1
fi

# Find clang-18
CLANG_18=$(which clang-18)
CLANGXX_18=$(which clang++-18)

if [ -z "$CLANG_18" ] || [ -z "$CLANGXX_18" ]; then
    echo "Error: clang-18 and clang++-18 must be in PATH"
    exit 1
fi

echo "Building LoongArch runtime library..."
echo "  GCC_TOOLCHAIN_PATH: $GCC_TOOLCHAIN_PATH"
echo "  CLANG_18: $CLANG_18"
echo "  CLANGXX_18: $CLANGXX_18"

cd "$SCRIPT_DIR"

# Compile flags matching NPB build
CXXFLAGS=(
    "--target=loongarch64-unknown-linux-gnu"
    "--gcc-toolchain=$GCC_TOOLCHAIN_PATH"
    "-fPIC"
    "-shared"
    "-O2"
)

if [ -d "$GCC_TOOLCHAIN_PATH/target" ]; then
    CXXFLAGS+=("--sysroot=$GCC_TOOLCHAIN_PATH/target")
fi

# Compile the library
echo "Compiling prefetcher_qemu_rt.cpp..."
"$CLANGXX_18" "${CXXFLAGS[@]}" -o libprefetcher_qemu_rt.so prefetcher_qemu_rt.cpp

if [ -f libprefetcher_qemu_rt.so ]; then
    echo "Success! Built libprefetcher_qemu_rt.so"
    file libprefetcher_qemu_rt.so
else
    echo "Error: Build failed"
    exit 1
fi
