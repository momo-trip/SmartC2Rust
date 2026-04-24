#!/bin/bash

# Initialize test counters
total_tests=20
passed_tests=0
current_test=0

# Clear and create results directory
rm -rf results
mkdir -p results


# Deterministic input generator (MUST match gen_expected.sh)
gen_input() {
    local size=$1
    if [ "$size" -eq 0 ]; then
        : > test_input
    else
        yes "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789" \
            | tr -d '\n' | head -c "$size" > test_input
    fi
}

# Function to run a test case and log its output
run_test() {
    local test_name=$1
    local input_size=$2
    local key_value=$3
    current_test=$((current_test + 1))
    
    # Log test start
    echo "Test Case #${current_test}: Started" | tee -a /root/SmartC2Rust/benchmark/SipHash/out_flow_c.log /root/SmartC2Rust/benchmark/SipHash/out_flow_rust.log
    
    # Create test input file (deterministic, matches gen_expected.sh)
    gen_input "$input_size"
    
    # Create test key file
    echo -n "$key_value" > test_key
    
    # Get expected output
    expected_output=$(cat "expected/test${current_test}.txt")
    
    # Run the test and capture actual output
    actual_output=$(./vectors test_input test_key 2>&1)
    
    # Compare outputs
    if [ "$actual_output" = "$expected_output" ]; then
        echo "Test #${current_test} passed"
        passed_tests=$((passed_tests + 1))
        log_file="results/test${current_test}_success.log"
    else
        echo "Test #${current_test} failed"
        log_file="results/test${current_test}_fail.log"
    fi
    
    # Log the output with comparison
    {
        echo "Test: $test_name"
        echo "Input size: $input_size bytes"
        echo "Key: $key_value"
        echo "Expected output:"
        echo "$expected_output"
        echo "Actual output:"
        echo "$actual_output"
        if [ "$actual_output" != "$expected_output" ]; then
            echo "Differences:"
            diff <(echo "$expected_output") <(echo "$actual_output")
        fi
    } > "$log_file"
    
    # Clean up
    rm -f test_input test_key
}

# Test cases with different input sizes and keys
run_test "Empty Input Test" 0 "testkey12345678"
run_test "Small Input Test" 1 "testkey12345678"
run_test "Medium Input Test" 32 "testkey12345678"
run_test "Large Input Test" 64 "testkey12345678"
run_test "Different Key Test 1" 16 "key1234567890123"
run_test "Different Key Test 2" 16 "differentkey1234"
run_test "Maximum Input Test" 64 "testkey12345678"
run_test "Binary Input Test" 16 "binary_key_test1"
run_test "Special Chars Test" 16 "special@key#123"
run_test "Numeric Key Test" 16 "123456789012345"
run_test "Boundary Test 1" 63 "testkey12345678"
run_test "Boundary Test 2" 2 "testkey12345678"
run_test "Mixed Input Test" 48 "mixedkey123test"
run_test "Sequential Input" 16 "sequential_key_1"
run_test "Unicode Key Test" 16 "unicode_key_テスト"
run_test "Long Input Test" 64 "long_key_test123"
run_test "Short Input Test" 4 "short_key_test1"
run_test "Random Input Test 1" 24 "random_key_test1"
run_test "Random Input Test 2" 40 "random_key_test2"
run_test "Edge Case Test" 64 "edge_case_key_12"

# Calculate pass rate
pass_rate=$((passed_tests * 100 / total_tests))

# Print results
echo "Tests completed: $total_tests"
echo "Tests passed: $passed_tests"
echo "Pass rate: $pass_rate%"

# If any tests failed, print which ones failed
if [ $pass_rate -ne 100 ]; then
    echo "Failed tests:" >&2
    for i in $(seq 1 $total_tests); do
        if [ -f "results/test${i}_fail.log" ]; then
            echo "Test #${i}" >&2
            cat "results/test${i}_fail.log" >&2
        fi
    done
    exit 1
fi

exit 0