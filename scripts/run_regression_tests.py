#!/usr/bin/env python3
"""
CudaGuard Regression Test Runner

Runs the cudaguard binary on regression test files and compares
the output against expected diagnostics.
"""

import os
import re
import subprocess
import sys
from pathlib import Path


def find_cudaguard_binary():
    """Locate the cudaguard binary in common build directories."""
    candidates = [
        "build/cudaguard",
        "build/Release/cudaguard",
        "build/Debug/cudaguard",
        "cmake-build-debug/cudaguard",
        "cmake-build-release/cudaguard",
    ]
    for candidate in candidates:
        if os.path.isfile(candidate):
            return candidate
        if os.path.isfile(candidate + ".exe"):
            return candidate + ".exe"

    # Try to find via environment variable
    env_path = os.environ.get("CUDAGUARD_BIN")
    if env_path and os.path.isfile(env_path):
        return env_path

    return None


def normalize_output(output):
    """
    Normalize diagnostic output for comparison.
    - Strip absolute paths, keep only filename
    - Remove line/column numbers (they may vary)
    - Keep only the rule ID and message
    """
    lines = []
    for line in output.strip().split('\n'):
        # Match diagnostic pattern: file:line:col: severity: CGXXX: message
        match = re.match(
            r'.*?:\d+:\d+:\s*(warning|error|note):\s*(CG\d{3}:\s*.+)',
            line
        )
        if match:
            lines.append(match.group(2).strip())
    return lines


def run_test(binary, test_file, expected_file):
    """Run cudaguard on a test file and compare with expected output."""
    cmd = [
        binary, "--file", str(test_file),
        "--", "-x", "cuda", "--cuda-gpu-arch=sm_75",
        "-nocudalib", "-nocudainc"
    ]

    try:
        result = subprocess.run(
            cmd,
            capture_output=True,
            text=True,
            timeout=30
        )
        output = result.stdout + result.stderr
    except subprocess.TimeoutExpired:
        return False, "TIMEOUT"
    except Exception as e:
        return False, f"EXECUTION ERROR: {e}"

    actual_diags = normalize_output(output)

    # Read expected diagnostics
    if not expected_file.exists():
        return False, f"Expected file not found: {expected_file}"

    expected_lines = []
    for line in expected_file.read_text().strip().split('\n'):
        line = line.strip()
        if line:
            expected_lines.append(line)

    # Compare
    if len(actual_diags) != len(expected_lines):
        return False, (
            f"Expected {len(expected_lines)} diagnostic(s), "
            f"got {len(actual_diags)}.\n"
            f"  Expected: {expected_lines}\n"
            f"  Actual:   {actual_diags}"
        )

    for i, (actual, expected) in enumerate(zip(actual_diags, expected_lines)):
        if expected not in actual:
            return False, (
                f"Diagnostic {i+1} mismatch:\n"
                f"  Expected (substring): {expected}\n"
                f"  Actual:               {actual}"
            )

    return True, "OK"


def run_good_test(binary, test_file):
    """Run cudaguard on a 'good' file and verify no diagnostics are reported."""
    cmd = [
        binary, "--file", str(test_file),
        "--", "-x", "cuda", "--cuda-gpu-arch=sm_75",
        "-nocudalib", "-nocudainc"
    ]

    try:
        result = subprocess.run(
            cmd,
            capture_output=True,
            text=True,
            timeout=30
        )
        output = result.stdout + result.stderr
    except subprocess.TimeoutExpired:
        return False, "TIMEOUT"
    except Exception as e:
        return False, f"EXECUTION ERROR: {e}"

    actual_diags = normalize_output(output)
    if actual_diags:
        return False, f"Expected no diagnostics but got: {actual_diags}"

    return True, "OK (no diagnostics)"


def main():
    script_dir = Path(__file__).parent
    project_root = script_dir.parent
    tests_dir = project_root / "tests" / "regression"
    bad_dir = tests_dir / "bad"
    good_dir = tests_dir / "good"
    expected_dir = tests_dir / "expected"

    binary = find_cudaguard_binary()
    if not binary:
        print("ERROR: Could not find cudaguard binary.")
        print("Build the project first: cmake --build build")
        print("Or set CUDAGUARD_BIN environment variable.")
        sys.exit(1)

    print(f"Using binary: {binary}")
    print(f"Test directory: {tests_dir}")
    print("=" * 60)

    passed = 0
    failed = 0
    total = 0

    # Run bad tests (should produce diagnostics)
    if bad_dir.exists():
        for test_file in sorted(bad_dir.glob("*.cu")):
            total += 1
            stem = test_file.stem
            expected_file = expected_dir / f"{stem}.txt"

            success, message = run_test(binary, test_file, expected_file)
            status = "PASS" if success else "FAIL"
            print(f"  [{status}] {test_file.name}: {message}")

            if success:
                passed += 1
            else:
                failed += 1

    # Run good tests (should produce no diagnostics)
    if good_dir.exists():
        for test_file in sorted(good_dir.glob("*.cu")):
            total += 1
            success, message = run_good_test(binary, test_file)
            status = "PASS" if success else "FAIL"
            print(f"  [{status}] {test_file.name}: {message}")

            if success:
                passed += 1
            else:
                failed += 1

    print("=" * 60)
    print(f"Results: {passed}/{total} passed, {failed} failed")

    return 0 if failed == 0 else 1


if __name__ == "__main__":
    sys.exit(main())
