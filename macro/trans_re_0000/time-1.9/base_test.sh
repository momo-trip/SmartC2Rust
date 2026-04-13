#!/bin/bash
# run_all_tests.sh - Run original test cases from tests directory
# Execute from: /home/ubuntu/programs/time_1_9

# Check if we're in the correct directory
if [ ! -f "./time" ] || [ ! -d "./tests" ]; then
    echo "Error: Please run this script from the time_1_9 directory"
    exit 1
fi

# Setup environment for tests
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
export PATH="$SCRIPT_DIR:$PATH"
export VERSION="1.9"
export srcdir="tests"
export builddir="."

echo "=== Running Original Test Cases ==="
echo "Working directory: $(pwd)"
echo "Time executable: ./time"
echo

# Change to tests directory
cd tests

# Add current tests directory to PATH so time-aux can be found
export PATH="$(pwd):$PATH"

# Add current tests directory to PATH so time-aux can be found
export PATH="$(pwd):$PATH"

# List of original test scripts
test_scripts=(
    "help-version.sh"
    "time-max-rss.sh"
    "time-exit-codes.sh"
    "time-posix-quiet.sh"
)

passed=0
failed=0

# Run each test script
for script in "${test_scripts[@]}"; do
    echo "Checking: $script"
    if [ -f "$script" ]; then
        echo "Running: $script"
        if ./"$script" > /dev/null 2>&1; then
            echo "  ✓ PASS"
            ((passed++))
        else
            echo "  ✗ FAIL"
            ((failed++))
        fi
    else
        echo "  ✗ SKIP (not found): $script"
        ((failed++))
    fi
done

echo
echo "=== Results ==="
echo "Passed: $passed"
echo "Failed: $failed"
echo "Total:  $((passed + failed))"

if [ $failed -eq 0 ]; then
    echo "All tests passed!"
    exit 0
else
    echo "Some tests failed!"
    exit 1
fi