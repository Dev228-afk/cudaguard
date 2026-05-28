#!/bin/bash
# Record a terminal demo GIF using 'asciinema' + 'agg' (or 'terminalizer').
#
# Prerequisites:
#   pip install asciinema
#   cargo install --git https://github.com/asciinema/agg  (for GIF conversion)
#
# Usage:
#   ./scripts/record_demo.sh
#
# This produces assets/demo.gif for embedding in the README.

set -e

PROJECT_ROOT="$(cd "$(dirname "$0")/.." && pwd)"
BINARY="${PROJECT_ROOT}/build/cudaguard"
OUTPUT_CAST="${PROJECT_ROOT}/assets/demo.cast"
OUTPUT_GIF="${PROJECT_ROOT}/assets/demo.gif"

if [ ! -f "$BINARY" ]; then
    echo "Error: Build cudaguard first (cmake --build build)"
    exit 1
fi

mkdir -p "${PROJECT_ROOT}/assets"

echo "Recording terminal demo..."

# Record with asciinema
asciinema rec "$OUTPUT_CAST" --command "bash -c '
echo \"$ cudaguard --file examples/demo/bad_host_device_call.cu -- -x cuda --cuda-gpu-arch=sm_75\"
sleep 0.5
${BINARY} --file ${PROJECT_ROOT}/examples/demo/bad_host_device_call.cu -- -x cuda --cuda-gpu-arch=sm_75 -nocudalib -nocudainc 2>&1 || true
echo
sleep 1

echo \"$ cudaguard --file examples/demo/bad_missing_error_check.cu -- -x cuda --cuda-gpu-arch=sm_75\"
sleep 0.5
${BINARY} --file ${PROJECT_ROOT}/examples/demo/bad_missing_error_check.cu -- -x cuda --cuda-gpu-arch=sm_75 -nocudalib -nocudainc 2>&1 || true
echo
sleep 1

echo \"$ cudaguard --json --file examples/demo/bad_memcpy_direction.cu -- -x cuda --cuda-gpu-arch=sm_75\"
sleep 0.5
${BINARY} --json --file ${PROJECT_ROOT}/examples/demo/bad_memcpy_direction.cu -- -x cuda --cuda-gpu-arch=sm_75 -nocudalib -nocudainc 2>&1 || true
echo
sleep 1

echo \"$ python3 scripts/run_regression_tests.py\"
sleep 0.5
cd ${PROJECT_ROOT} && python3 scripts/run_regression_tests.py 2>&1 | head -20
echo \"...\"
sleep 1
'" --overwrite --title "CudaGuard Demo"

echo "Converting to GIF..."

# Convert to GIF (requires agg)
if command -v agg &> /dev/null; then
    agg "$OUTPUT_CAST" "$OUTPUT_GIF" --cols 100 --rows 30 --speed 1.5
    echo "Done: ${OUTPUT_GIF}"
else
    echo "Cast recorded: ${OUTPUT_CAST}"
    echo "Install 'agg' to convert to GIF: cargo install --git https://github.com/asciinema/agg"
    echo "Then run: agg ${OUTPUT_CAST} ${OUTPUT_GIF} --cols 100 --rows 30 --speed 1.5"
fi
