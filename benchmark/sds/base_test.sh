#!/bin/bash

# Create results directory
rm -rf results
mkdir -p results

# Initialize test counters
total_tests=0
passed_tests=0
current_test=0

# Path to the SDS test program
SDS_TEST_PROGRAM="./sds-test"

# Function to log test start
log_test_start() {
    echo "Test Case #$1: Started" | tee -a /home/ubuntu/portable/out_flow_c.log /home/ubuntu/portable/out_flow_rust.log
}

# Hardcoded test case names
test_cases=(
    "create_and_length"
    "create_with_specified_length"
    "string_concatenation"
    "sdscpy_against_longer_string"
    "sdscpy_against_shorter_string"
    "sdscatprintf_base_case"
    "sdscatprintf_with_null_char"
    "sdscatprintf_large_string"
    "sdscatfmt_base_case"
    "sdscatfmt_unsigned_numbers"
    "sdstrim_all_chars_match"
    "sdstrim_single_char_remains"
    "sdstrim_correctly_trims"
    "sdsrange_single_char"
    "sdsrange_from_start"
    "sdsrange_from_end"
    "sdsrange_empty_result"
    "sdsrange_beyond_length"
    "sdsrange_out_of_bounds"
    "sdscmp_greater"
    "sdscmp_equal"
    "sdscmp_less"
    "sdscatrepr"
    "sdsmakeroomfor"
)
total_tests=${#test_cases[@]}


# Run each test
for test_name in "${test_cases[@]}"; do
    ((current_test++))
    log_test_start $current_test "$test_name"
    
    # Run the test and capture exit code
    $SDS_TEST_PROGRAM "$test_name" > "results/test${current_test}_output.log" 2>&1
    exit_code=$?
    
    if [ $exit_code -eq 0 ]; then
        echo "Test #${current_test}: $test_name - PASSED"
        echo "Test passed: $test_name" > "results/test${current_test}_success.log"
        ((passed_tests++))
    else
        echo "Test #${current_test}: $test_name - FAILED" >&2
        echo "Test failed: $test_name with exit code $exit_code" > "results/test${current_test}_fail.log"
    fi
done

# Calculate pass rate
pass_rate=$((passed_tests * 100 / total_tests))
echo "-------------------------"
echo "Pass rate: ${pass_rate}% ($passed_tests of $total_tests tests passed)"

# Output failed tests if any
if [ $pass_rate -ne 100 ]; then
    echo "Failed tests:" >&2
    for i in $(seq 1 $total_tests); do
        if [ -f "results/test${i}_fail.log" ]; then
            failed_test=$(sed 's/Test failed: \(.*\) with.*/\1/' "results/test${i}_fail.log")
            echo "Test #${i}: $failed_test" >&2
        fi
    done
    exit 1
fi

echo "All tests passed!"
exit 0