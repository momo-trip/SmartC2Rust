
#!/bin/bash

# Reformed test cases

failed=0
mkdir -p flow_results

run_test_case() {
    local test_num=$1
    local test_array=$2
    local expected=$3
    local test_name=$4

    echo "Test ${test_num} started"

    local binary="./bin/tests/qsort_test_t${test_num}"
    if [ ! -x "$binary" ]; then
        echo "Test ${test_num} failed" >&2
        echo "Binary $binary not found or not executable" > "flow_results/test${test_num}_fail.log"
        failed=1
        echo "Test ${test_num} ended"
        return
    fi

    local output
    output=$(LD_PRELOAD=libtracer.so TRACE_OUTPUT=$PWD/flow_results/test${test_num}_trace.log $binary $test_num $test_array 2>&1)
    local exit_code=$?

    local sorted_line
    sorted_line=$(echo "$output" | grep "After sorting:" | awk -F "After sorting: " '{print $2}')

    if [ "$sorted_line" = "$expected" ]; then
        echo "Test ${test_num} passed"
        {
            echo "Command: $binary $test_num $test_array"
            echo "Output: $output"
            echo "Expected sorted array: $expected"
            echo "Actual sorted array: $sorted_line"
            echo "Exit code: $exit_code"
        } > "flow_results/test${test_num}_success.log"
    else
        echo "Test ${test_num} failed" >&2
        {
            echo "Command: $binary $test_num $test_array"
            echo "Output: $output"
            echo "Expected sorted array: $expected"
            echo "Actual sorted array: $sorted_line"
            echo "Exit code: $exit_code"
        } > "flow_results/test${test_num}_fail.log"
        failed=1
    fi

    echo "Test ${test_num} ended"
}

run_test_case 1 "5 2 8 1 3" "1 2 3 5 8 " "Sort an array in ascending order"
run_test_case 2 "4 1 3 4 2 2" "1 2 2 3 4 4 " "Sort an array with repeated values"
run_test_case 3 "1 2 3 4 5" "1 2 3 4 5 " "Sort an already sorted array"
run_test_case 4 "42" "42 " "Sort an array with one element"
run_test_case 5 "" "" "Sort an empty array"
run_test_case 6 "-5 12 -10 0 8 -15 3" "-15 -10 -5 0 3 8 12 " "Array with negative numbers"
run_test_case 7 "" "" "Empty array"
run_test_case 8 "45 23 68 12 90 34 56 78 11 99 22 33 44 55 66 77 88 98 21 32" "11 12 21 22 23 32 33 34 44 45 55 56 66 68 77 78 88 90 98 99 " "Large array (20 elements)"
run_test_case 9 "0 0 0 0 0" "0 0 0 0 0 " "Array with all zeros"
run_test_case 10 "42 42 42 42 42" "42 42 42 42 42 " "Array with all the same value"
run_test_case 11 "1 2 1 2 1 2" "1 1 1 2 2 2 " "Array with alternating values"
run_test_case 12 "2147483647 -2147483648 0" "-2147483648 0 2147483647 " "Array with extreme values"
run_test_case 13 "1 2 3 5 4 6" "1 2 3 4 5 6 " "Near-sorted array"
run_test_case 14 "100 1 100 1 100 1" "1 1 1 100 100 100 " "Array with repeated min/max values"
run_test_case 15 "42 17 35 9 24" "9 17 24 35 42 " "Small random array"
run_test_case 16 "2 -4 6 -8 10 -12" "-12 -8 -4 2 6 10 " "Mix of positive and negative even numbers"
run_test_case 17 "1 -3 5 -7 9 -11" "-11 -7 -3 1 5 9 " "Mix of positive and negative odd numbers"
run_test_case 18 "5 6 3 4 1 2 7 8" "1 2 3 4 5 6 7 8 " "Array with sequential pairs"
run_test_case 19 "1 3 5 7 5 3 1" "1 1 3 3 5 5 7 " "Palindromic array"
run_test_case 20 "5 1" "1 5 " "Two-element array"

exit $failed

