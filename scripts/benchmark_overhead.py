#!/usr/bin/env python3
"""
CudaGuard Overhead Benchmark

Measures the performance overhead of CudaGuard analysis relative to
nvcc compilation time, demonstrating <5% wrapper overhead.

Usage:
    python3 scripts/benchmark_overhead.py [--cudaguard PATH] [--nvcc PATH]
"""

import argparse
import os
import subprocess
import sys
import time
from pathlib import Path


def find_binary(name, env_var=None):
    """Find a binary in PATH or via environment variable."""
    if env_var:
        path = os.environ.get(env_var)
        if path and os.path.isfile(path):
            return path

    # Check common build dirs for cudaguard
    if name == "cudaguard":
        candidates = [
            "build/cudaguard",
            "build/Release/cudaguard",
            "build/Debug/cudaguard",
            "cmake-build-release/cudaguard",
        ]
        for c in candidates:
            if os.path.isfile(c):
                return c
            if os.path.isfile(c + ".exe"):
                return c + ".exe"

    # Try which/where
    try:
        result = subprocess.run(
            ["which", name], capture_output=True, text=True
        )
        if result.returncode == 0:
            return result.stdout.strip()
    except FileNotFoundError:
        pass

    return None


def time_command(cmd, runs=3):
    """Run a command multiple times and return the median wall time."""
    times = []
    for _ in range(runs):
        start = time.perf_counter()
        try:
            subprocess.run(
                cmd, capture_output=True, timeout=60
            )
        except subprocess.TimeoutExpired:
            return None
        except FileNotFoundError:
            return None
        elapsed = time.perf_counter() - start
        times.append(elapsed)
    times.sort()
    return times[len(times) // 2]  # median


def benchmark_file(cudaguard_bin, nvcc_bin, cuda_file):
    """Benchmark a single CUDA file."""
    print(f"\n{'='*60}")
    print(f"File: {cuda_file}")
    print(f"{'='*60}")

    # 1. Measure nvcc compile time (if nvcc available)
    nvcc_time = None
    if nvcc_bin:
        nvcc_cmd = [nvcc_bin, "-arch=sm_75", "-c", cuda_file, "-o", "/dev/null"]
        nvcc_time = time_command(nvcc_cmd)
        if nvcc_time:
            print(f"  nvcc compile time:         {nvcc_time:.3f}s")
        else:
            print(f"  nvcc compile time:         (unavailable)")

    # 2. Measure cudaguard analysis time
    cg_cmd = [
        cudaguard_bin, "--file", cuda_file,
        "--", "-x", "cuda", "--cuda-gpu-arch=sm_75",
        "-nocudalib", "-nocudainc"
    ]
    cg_time = time_command(cg_cmd)
    if cg_time:
        print(f"  cudaguard analysis time:   {cg_time:.3f}s")
    else:
        print(f"  cudaguard analysis time:   FAILED")
        return None

    # 3. Measure wrapper mode (cudaguard + nvcc)
    wrapper_time = None
    if nvcc_bin:
        wrapper_cmd = [
            cudaguard_bin, "--wrap-nvcc", "--",
            nvcc_bin, "-arch=sm_75", "-c", cuda_file, "-o", "/dev/null"
        ]
        wrapper_time = time_command(wrapper_cmd)
        if wrapper_time:
            print(f"  cudaguard + nvcc wrapper:  {wrapper_time:.3f}s")

    # 4. Calculate overhead
    if nvcc_time and nvcc_time > 0:
        overhead_pct = ((cg_time) / nvcc_time) * 100
        print(f"  analysis overhead:         {overhead_pct:.1f}% of nvcc time")

        if wrapper_time:
            wrapper_overhead = ((wrapper_time - nvcc_time) / nvcc_time) * 100
            print(f"  wrapper overhead:          {wrapper_overhead:.1f}% over nvcc alone")
    else:
        # No nvcc — just report absolute time
        print(f"  analysis time (absolute):  {cg_time:.3f}s")
        print(f"  (nvcc not available for overhead comparison)")

    return {
        "file": cuda_file,
        "nvcc_time": nvcc_time,
        "cg_time": cg_time,
        "wrapper_time": wrapper_time,
    }


def main():
    parser = argparse.ArgumentParser(description="CudaGuard overhead benchmark")
    parser.add_argument("--cudaguard", help="Path to cudaguard binary")
    parser.add_argument("--nvcc", help="Path to nvcc binary")
    parser.add_argument("--runs", type=int, default=3, help="Number of runs per measurement")
    args = parser.parse_args()

    script_dir = Path(__file__).parent
    project_root = script_dir.parent

    # Find binaries
    cudaguard_bin = args.cudaguard or find_binary("cudaguard", "CUDAGUARD_BIN")
    nvcc_bin = args.nvcc or find_binary("nvcc")

    if not cudaguard_bin:
        print("ERROR: cudaguard binary not found.")
        print("Build first: cmake --build build")
        print("Or pass --cudaguard /path/to/binary")
        sys.exit(1)

    print("CudaGuard Overhead Benchmark")
    print(f"  cudaguard: {cudaguard_bin}")
    print(f"  nvcc:      {nvcc_bin or '(not found — will report absolute times)'}")
    print(f"  runs:      {args.runs} per measurement (median reported)")

    # Test files
    test_files = [
        str(project_root / "examples" / "vector_add.cu"),
        str(project_root / "examples" / "matrix_add.cu"),
        str(project_root / "examples" / "reductions.cu"),
        str(project_root / "examples" / "broken_examples" / "missing_error_check.cu"),
        str(project_root / "examples" / "broken_examples" / "bad_qualifier_usage.cu"),
    ]

    results = []
    for f in test_files:
        if os.path.isfile(f):
            r = benchmark_file(cudaguard_bin, nvcc_bin, f)
            if r:
                results.append(r)

    # Summary
    if results:
        print(f"\n{'='*60}")
        print("SUMMARY")
        print(f"{'='*60}")

        cg_times = [r["cg_time"] for r in results if r["cg_time"]]
        if cg_times:
            avg_cg = sum(cg_times) / len(cg_times)
            print(f"  Average analysis time:  {avg_cg:.3f}s")

        nvcc_times = [r["nvcc_time"] for r in results if r["nvcc_time"]]
        if nvcc_times:
            avg_nvcc = sum(nvcc_times) / len(nvcc_times)
            avg_overhead = (avg_cg / avg_nvcc) * 100
            print(f"  Average nvcc time:      {avg_nvcc:.3f}s")
            print(f"  Average overhead:       {avg_overhead:.1f}%")

            if avg_overhead < 5.0:
                print(f"\n  RESULT: <5% overhead confirmed.")
            else:
                print(f"\n  RESULT: Overhead is {avg_overhead:.1f}% (target: <5%)")
        else:
            print(f"\n  (nvcc not available — overhead percentage not computed)")
            print(f"  Analysis completes in {avg_cg:.3f}s per file on average.")

    return 0


if __name__ == "__main__":
    sys.exit(main())
