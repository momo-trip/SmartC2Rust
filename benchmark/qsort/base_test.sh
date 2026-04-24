#!/bin/bash
# [1]: https://github.com/bradtraversy/traversy-js-challenges/blob/main/09-sorting-algorithms/11-quick-sort-implementation/quick-sort-test.js

# Initialize test counters
total_tests=20
passed_tests=0
current_test=0

# Create results directory
rm -rf results
mkdir -p results

# Function to log test start
log_test_start() {
    echo "Test Case #$1: Started" | tee -a /root/SmartC2Rust/benchmark/qsort/out_flow_c.log /root/SmartC2Rust/benchmark/qsort/out_flow_rust.log
}

# Function to run a test and check its output
run_test() {
    current_test=$1
    test_array=$2
    expected_sorted_array=$3
    test_name=$4
    
    echo "Test Case #${current_test}: ${test_name}"
    log_test_start $current_test "$test_name"
    
    # Run the test command and capture output
    output=$(./bin/qsort_test $current_test $test_array 2>&1)
    exit_code=$?
    
    # Extract the "After sorting" line from output
    sorted_line=$(echo "$output" | grep "After sorting:" | awk -F "After sorting: " '{print $2}')
    
    # Compare the sorted output with expected
    if [ "$sorted_line" = "$expected_sorted_array" ]; then
        echo "Test #${current_test} passed"
        passed_tests=$((passed_tests + 1))
        log_file="results/test${current_test}_success.log"
    else
        echo "Test #${current_test} failed"
        log_file="results/test${current_test}_fail.log"
    fi
    
    # Log the output
    echo "Command: ./bin/qsort_test $current_test $test_array" > "$log_file"
    echo "Output: $output" >> "$log_file"
    echo "Expected sorted array: $expected_sorted_array" >> "$log_file"
    echo "Actual sorted array: $sorted_line" >> "$log_file"
    echo "Exit code: $exit_code" >> "$log_file"
}

# Test cases

# Test 1: Sort an array in ascending order (from [1])
run_test 1 "5 2 8 1 3" "1 2 3 5 8 " "Sort an array in ascending order"

# Test 2: Sort an array with repeated values (from [1])
run_test 2 "4 1 3 4 2 2" "1 2 2 3 4 4 " "Sort an array with repeated values"

# Test 3: Sort an already sorted array (from [1])
run_test 3 "1 2 3 4 5" "1 2 3 4 5 " "Sort an already sorted array"

# Test 4: Sort an array with one element (from [1])
run_test 4 "42" "42 " "Sort an array with one element"

# Test 5: Sort an empty array (from [1])
run_test 5 "" "" "Sort an empty array"

# Test 6: Array with negative numbers (additional)
run_test 6 "-5 12 -10 0 8 -15 3" "-15 -10 -5 0 3 8 12 " "Array with negative numbers"

# Test 7: Empty array - should not crash (additional)
run_test 7 "" "" "Empty array"

# Test 8: Large array (20 elements) (additional)
run_test 8 "45 23 68 12 90 34 56 78 11 99 22 33 44 55 66 77 88 98 21 32" "11 12 21 22 23 32 33 34 44 45 55 56 66 68 77 78 88 90 98 99 " "Large array (20 elements)"

# Test 9: Array with all zeros (additional)
run_test 9 "0 0 0 0 0" "0 0 0 0 0 " "Array with all zeros"

# Test 10: Array with all the same value (additional)
run_test 10 "42 42 42 42 42" "42 42 42 42 42 " "Array with all the same value"

# Test 11: Array with alternating values (additional)
run_test 11 "1 2 1 2 1 2" "1 1 1 2 2 2 " "Array with alternating values"

# Test 12: Array with extreme values (additional)
run_test 12 "2147483647 -2147483648 0" "-2147483648 0 2147483647 " "Array with extreme values"

# Test 13: Near-sorted array (only one element out of place) (additional)
run_test 13 "1 2 3 5 4 6" "1 2 3 4 5 6 " "Near-sorted array"

# Test 14: Array with repeated min/max values (additional)
run_test 14 "100 1 100 1 100 1" "1 1 1 100 100 100 " "Array with repeated min/max values"

# Test 15: Small random array (additional)
run_test 15 "42 17 35 9 24" "9 17 24 35 42 " "Small random array"

# Test 16: Mix of positive and negative even numbers (additional)
run_test 16 "2 -4 6 -8 10 -12" "-12 -8 -4 2 6 10 " "Mix of positive and negative even numbers"

# Test 17: Mix of positive and negative odd numbers (additional)
run_test 17 "1 -3 5 -7 9 -11" "-11 -7 -3 1 5 9 " "Mix of positive and negative odd numbers"

# Test 18: Array with sequential pairs (additional)
run_test 18 "5 6 3 4 1 2 7 8" "1 2 3 4 5 6 7 8 " "Array with sequential pairs"

# Test 19: Palindromic array (additional)
run_test 19 "1 3 5 7 5 3 1" "1 1 3 3 5 5 7 " "Palindromic array"

# Test 20: Two-element array (edge case) (additional)
run_test 20 "5 1" "1 5 " "Two-element array"

# Calculate pass rate
pass_rate=$((passed_tests * 100 / total_tests))
echo "Pass rate: ${pass_rate}%"

# Output failed tests if any
if [ $pass_rate -ne 100 ]; then
    echo "Failed tests:" >&2
    for i in $(seq 1 $total_tests); do
        if [ -f "results/test${i}_fail.log" ]; then
            echo "Test #${i} failed" >&2
        fi
    done
    exit 1
fi

exit 0