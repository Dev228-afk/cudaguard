#!/bin/bash
# CudaGuard Demo Script
# Demonstrates the tool's capabilities in under 2 minutes.

set -e

PROJECT_ROOT="$(cd "$(dirname "$0")/.." && pwd)"
BINARY="${PROJECT_ROOT}/build/cudaguard"

if [ ! -f "$BINARY" ]; then
    echo "Error: cudaguard binary not found at ${BINARY}"
    echo "Build the project first:"
    echo "  cmake -S . -B build -DCMAKE_EXPORT_COMPILE_COMMANDS=ON"
    echo "  cmake --build build"
    exit 1
fi

echo "============================================================"
echo "CudaGuard Demo - CUDA C++ Static Analysis & Build Diagnostics"
echo "============================================================"
echo ""

echo "--- Example 1: Missing CUDA Error Check (CG001) ---"
echo "$ cudaguard --file examples/broken_examples/missing_error_check.cu -- -x cuda"
echo ""
"$BINARY" --file "${PROJECT_ROOT}/examples/broken_examples/missing_error_check.cu" \
    -- -x cuda --cuda-gpu-arch=sm_75 -nocudalib -nocudainc || true
echo ""

echo "--- Example 2: Suspicious Kernel Launch Config (CG002) ---"
echo "$ cudaguard --file examples/broken_examples/wrong_grid_config.cu -- -x cuda"
echo ""
"$BINARY" --file "${PROJECT_ROOT}/examples/broken_examples/wrong_grid_config.cu" \
    -- -x cuda --cuda-gpu-arch=sm_75 -nocudalib -nocudainc || true
echo ""

echo "--- Example 3: Host/Device Qualifier Misuse (CG003) ---"
echo "$ cudaguard --file examples/broken_examples/bad_qualifier_usage.cu -- -x cuda"
echo ""
"$BINARY" --file "${PROJECT_ROOT}/examples/broken_examples/bad_qualifier_usage.cu" \
    -- -x cuda --cuda-gpu-arch=sm_75 -nocudalib -nocudainc || true
echo ""

echo "--- Example 4: JSON Output ---"
echo "$ cudaguard --json --file examples/broken_examples/wrong_grid_config.cu -- -x cuda"
echo ""
"$BINARY" --json --file "${PROJECT_ROOT}/examples/broken_examples/wrong_grid_config.cu" \
    -- -x cuda --cuda-gpu-arch=sm_75 -nocudalib -nocudainc || true
echo ""

echo "--- Example 5: Clean File (No Diagnostics) ---"
echo "$ cudaguard --file examples/vector_add.cu -- -x cuda"
echo ""
"$BINARY" --file "${PROJECT_ROOT}/examples/vector_add.cu" \
    -- -x cuda --cuda-gpu-arch=sm_75 -nocudalib -nocudainc || true
echo ""

echo "--- Running Regression Tests ---"
echo "$ python3 scripts/run_regression_tests.py"
echo ""
cd "$PROJECT_ROOT"
python3 scripts/run_regression_tests.py || true
echo ""

echo "============================================================"
echo "Demo complete."
echo "============================================================"
