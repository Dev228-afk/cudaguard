#!/bin/bash
# Generate compile_commands.json for CudaGuard example files
# This creates a compilation database that cudaguard can consume.

set -e

PROJECT_ROOT="$(cd "$(dirname "$0")/.." && pwd)"
BUILD_DIR="${PROJECT_ROOT}/build"

echo "Generating compile_commands.json..."

# Option 1: Use CMake to generate it (preferred)
if command -v cmake &> /dev/null; then
    cmake -S "${PROJECT_ROOT}" -B "${BUILD_DIR}" -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
    echo "Generated: ${BUILD_DIR}/compile_commands.json"
    exit 0
fi

# Option 2: Generate manually for example files
COMPILE_DB="${BUILD_DIR}/compile_commands.json"
mkdir -p "${BUILD_DIR}"

echo "[" > "${COMPILE_DB}"

first=true
for cu_file in "${PROJECT_ROOT}"/examples/*.cu "${PROJECT_ROOT}"/examples/broken_examples/*.cu; do
    if [ ! -f "$cu_file" ]; then
        continue
    fi

    if [ "$first" = true ]; then
        first=false
    else
        echo "," >> "${COMPILE_DB}"
    fi

    cat >> "${COMPILE_DB}" << EOF
  {
    "directory": "${PROJECT_ROOT}",
    "command": "clang++ -x cuda --cuda-gpu-arch=sm_75 -c ${cu_file}",
    "file": "${cu_file}"
  }
EOF
done

echo "]" >> "${COMPILE_DB}"

echo "Generated: ${COMPILE_DB}"
echo "Contains entries for $(grep -c '"file"' "${COMPILE_DB}") files."
