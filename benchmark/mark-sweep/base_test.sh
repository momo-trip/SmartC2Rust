#!/bin/bash

# Create results directory
rm -rf results
mkdir -p results

# Initialize test counters
total_tests=5
passed_tests=0
current_test=0

# Function to log test start
log_test_start() {
    echo "Test Case #$1: Started"
}

if [ ! -f ./markandsweep ]; then
    echo "Build failed" >&2
    exit 1
fi

# Test 1: Run test case 1
current_test=1
log_test_start $current_test
output=$(./markandsweep 1 2>&1)
expected_output=$(cat expected/test1.log)
if [ "$output" == "$expected_output" ]; then
    echo "Test #${current_test} passed"
    echo "$output" > "results/test${current_test}_success.log"
    ((passed_tests++))
else
    echo "Test #${current_test} failed" >&2
    echo "$output" > "results/test${current_test}_fail.log"
    diff -u expected/test1.log <(echo "$output") > "results/test${current_test}_diff.log"
fi

# Test 2: Run test case 2
current_test=2
log_test_start $current_test
output=$(./markandsweep 2 2>&1)
expected_output=$(cat expected/test2.log)
if [ "$output" == "$expected_output" ]; then
    echo "Test #${current_test} passed"
    echo "$output" > "results/test${current_test}_success.log"
    ((passed_tests++))
else
    echo "Test #${current_test} failed" >&2
    echo "$output" > "results/test${current_test}_fail.log"
    diff -u expected/test2.log <(echo "$output") > "results/test${current_test}_diff.log"
fi

# Test 3: Run test case 3
current_test=3
log_test_start $current_test
output=$(./markandsweep 3 2>&1)
expected_output=$(cat expected/test3.log)
if [ "$output" == "$expected_output" ]; then
    echo "Test #${current_test} passed"
    echo "$output" > "results/test${current_test}_success.log"
    ((passed_tests++))
else
    echo "Test #${current_test} failed" >&2
    echo "$output" > "results/test${current_test}_fail.log"
    diff -u expected/test3.log <(echo "$output") > "results/test${current_test}_diff.log"
fi

# Test 4: Run test case 4
current_test=4
log_test_start $current_test
output=$(./markandsweep 4 2>&1)
expected_output=$(cat expected/test4.log)
if [ "$output" == "$expected_output" ]; then
    echo "Test #${current_test} passed"
    echo "$output" > "results/test${current_test}_success.log"
    ((passed_tests++))
else
    echo "Test #${current_test} failed" >&2
    echo "$output" > "results/test${current_test}_fail.log"
    diff -u expected/test4.log <(echo "$output") > "results/test${current_test}_diff.log"
fi

# Test 5: Run test case 5 (performance test)
current_test=5
log_test_start $current_test
output=$(./markandsweep 5 2>&1)
expected_output=$(cat expected/test5.log)
if [ "$output" == "$expected_output" ]; then
    echo "Test #${current_test} passed"
    echo "$output" > "results/test${current_test}_success.log"
    ((passed_tests++))
else
    echo "Test #${current_test} failed" >&2
    echo "$output" > "results/test${current_test}_fail.log"
    diff -u expected/test5.log <(echo "$output") > "results/test${current_test}_diff.log"
fi


# Calculate pass rate
pass_rate=$((passed_tests * 100 / total_tests))
echo "Pass rate: ${pass_rate}%"

# Output failed tests if any
if [ $pass_rate -ne 100 ]; then
    echo "Failed tests:" >&2
    for i in $(seq 1 $total_tests); do
        if [ -f "results/test${i}_fail.log" ]; then
            echo "Test #${i}" >&2
        fi
    done
    exit 1
fi

exit 0