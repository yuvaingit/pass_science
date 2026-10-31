#!/usr/bin/env bash
set -e

# PASS SCIENCE WORKS - One-Click Launcher (macOS/Linux)
echo "========================================================"
echo "  Starting PASS Science Works Unified Engineering Workbench"
echo "========================================================"

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$SCRIPT_DIR"

if [ ! -f "build/bin/workbench_server" ]; then
    echo "[INFO] Building C++ backend using CMake..."
    cmake -B build
    cmake --build build
fi

echo "[INFO] Launching workbench server..."
./build/bin/workbench_server
