#!/bin/bash

# Color definitions
GREEN='\033[0;32m'
RED='\033[0;31m'
NC='\033[0m' # No Color

# Directories for test results
ACTUAL_DIR="actual"
EXPECTED_DIR="expected"
RESULTS_DIR="results"

# Create directories if they don't exist
rm -rf ${ACTUAL_DIR}
rm -rf ${RESULTS_DIR}
mkdir -p ${ACTUAL_DIR}
mkdir -p ${RESULTS_DIR}

# Test function
run_test() {
    local test_num=$1
    local build_args=$2

    local output_file="${ACTUAL_DIR}/test${test_num}_output.log"
    local expected_file="${EXPECTED_DIR}/test${test_num}_output.log"
    local success_log="${RESULTS_DIR}/test${test_num}_success.log"
    local fail_log="${RESULTS_DIR}/test${test_num}_fail.log"
    
    echo "Running test case ${test_num}..."
    
    # Run
    ./test.elf > "${output_file}" 2>&1
    
    # Check if each line in the expected file is included as a prefix in the corresponding line of the output file
    local failed=0
    local line_num=0
    local failure_details=""
    
    # Guard: expected file must exist; otherwise the comparison loop silently passes.
    if [ ! -f "${expected_file}" ]; then
        echo -e "${RED}Expected file not found: ${expected_file}${NC}" >&2
        failure_details="Expected file not found: ${expected_file}\n"
        failed=1
    fi

    while IFS= read -r expected_line; do
        line_num=$((line_num + 1))
        
        # Skip empty lines
        [ -z "$expected_line" ] && continue
        
        # Get the corresponding line from the actual output
        actual_line=$(sed -n "${line_num}p" "${output_file}")
        
        # Check if the expected value is included as a prefix in the actual output
        if [[ ! "$actual_line" == "$expected_line"* ]]; then
            failed=1
            echo -e "${RED}Line ${line_num} does not match${NC}"
            echo "Expected: $expected_line"
            echo "Actual: $actual_line"
            
            # Collect failure details
            failure_details="${failure_details}Line ${line_num} does not match\nExpected: ${expected_line}\nActual: ${actual_line}\n\n"
        fi
    done < "${expected_file}"
    
    if [ $failed -eq 0 ]; then
        echo -e "${GREEN}Test case ${test_num}: SUCCESS${NC}"
        echo "Test #${test_num} passed"
        echo "AES test case ${test_num} with args '${build_args}' passed successfully" > "${success_log}"
        return 0
    else
        echo -e "${RED}Test case ${test_num}: FAILURE${NC}"
        echo "Test #${test_num} failed" >&2
        echo -e "AES test case ${test_num} with args '${build_args}' failed:\n\n${failure_details}" > "${fail_log}"
        return 1
    fi
}

# Main process
echo "Starting AES implementation verification tests..."

# Test result counters
total_tests=1 #3
passed_tests=0

# Test case 1: Default AES
run_test 1 ""
if [ $? -eq 0 ]; then
    ((passed_tests++))
fi

# Result summary
echo ""
echo "Test result summary: ${passed_tests}/${total_tests} tests passed"

if [ ${passed_tests} -eq ${total_tests} ]; then
    echo -e "${GREEN}All tests passed!${NC}"
    echo "All tests passed!"
    exit 0
else
    echo -e "${RED}Some tests failed.${NC}"
    echo "Some tests failed: ${passed_tests}/${total_tests} passed"
    exit 1
fi