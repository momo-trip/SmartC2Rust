#!/bin/bash

RESULTS_DIR="test-results"
EXPECTED_DIR="expected"

# Initialize test counters
total_tests=20
passed_tests=0
current_test=0

# Cleanup
rm -rf ${RESULTS_DIR}
mkdir -p ${EXPECTED_DIR}

gen_input() {
    local size=$1
    if [ "$size" -eq 0 ]; then
        : > test_input
    else
        yes "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789" \
            | tr -d '\n' | head -c "$size" > test_input
    fi
}

gen_expected() {
    local test_name=$1
    local input_size=$2
    local key_value=$3
    current_test=$((current_test + 1))
    
    # Generate input files (fixed seed)
    gen_input "$input_size"
    echo -n "$key_value" > test_key
    ./vectors test_input test_key > "${EXPECTED_DIR}/test${current_test}.txt" 2>/dev/null
    
}

echo "Generating expected values..."
current_test=0
gen_expected "Empty Input Test" 0 "testkey12345678"
gen_expected "Small Input Test" 1 "testkey12345678"
gen_expected "Medium Input Test" 32 "testkey12345678"
gen_expected "Large Input Test" 64 "testkey12345678"
gen_expected "Different Key Test 1" 16 "key1234567890123"
gen_expected "Different Key Test 2" 16 "differentkey1234"
gen_expected "Maximum Input Test" 64 "testkey12345678"
gen_expected "Binary Input Test" 16 "binary_key_test1"
gen_expected "Special Chars Test" 16 "special@key#123"
gen_expected "Numeric Key Test" 16 "123456789012345"
gen_expected "Boundary Test 1" 63 "testkey12345678"
gen_expected "Boundary Test 2" 2 "testkey12345678"
gen_expected "Mixed Input Test" 48 "mixedkey123test"
gen_expected "Sequential Input" 16 "sequential_key_1"
gen_expected "Unicode Key Test" 16 "unicode_key_テスト"
gen_expected "Long Input Test" 64 "long_key_test123"
gen_expected "Short Input Test" 4 "short_key_test1"
gen_expected "Random Input Test 1" 24 "random_key_test1"
gen_expected "Random Input Test 2" 40 "random_key_test2"
gen_expected "Edge Case Test" 64 "edge_case_key_12"

