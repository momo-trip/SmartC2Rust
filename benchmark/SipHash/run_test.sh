
#!/bin/bash

# Reformed test cases

failed=0
mkdir -p flow_results

# Deterministic input generator (MUST match get_expected.sh)
gen_input() {
    local size=$1
    if [ "$size" -eq 0 ]; then
        : > test_input
    else
        yes "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789" \
            | tr -d '\n' | head -c "$size" > test_input
    fi
}

run_test() {
    local test_num=$1
    local test_name=$2
    local input_size=$3
    local key_value=$4

    echo "Test ${test_num} started"

    local binary="./test_t${test_num}"
    local trace_log="$PWD/flow_results/test${test_num}_trace.log"
    local success_log="flow_results/test${test_num}_success.log"
    local fail_log="flow_results/test${test_num}_fail.log"
    local input_file="test_input_${test_num}"
    local key_file="test_key_${test_num}"

    # Generate deterministic input
    if [ "$input_size" -eq 0 ]; then
        : > "$input_file"
    else
        yes "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789" \
            | tr -d '\n' | head -c "$input_size" > "$input_file"
    fi
    echo -n "$key_value" > "$key_file"

    expected_output=$(cat "expected/test${test_num}.txt")
    actual_output=$(LD_PRELOAD=libtracer.so TRACE_OUTPUT="$trace_log" "$binary" "$input_file" "$key_file" 2>&1)

    if [ "$actual_output" = "$expected_output" ]; then
        {
            echo "Test: $test_name"
            echo "Input size: $input_size bytes"
            echo "Key: $key_value"
            echo "Expected output:"
            echo "$expected_output"
            echo "Actual output:"
            echo "$actual_output"
        } > "$success_log"
        echo "Test ${test_num} passed"
    else
        {
            echo "Test: $test_name"
            echo "Input size: $input_size bytes"
            echo "Key: $key_value"
            echo "Expected output:"
            echo "$expected_output"
            echo "Actual output:"
            echo "$actual_output"
            echo "Differences:"
            diff <(echo "$expected_output") <(echo "$actual_output")
        } > "$fail_log"
        echo "Test ${test_num} failed" >&2
        failed=1
    fi

    rm -f "$input_file" "$key_file"
    echo "Test ${test_num} ended"
}

run_test 1  "Empty Input Test"       0  "testkey12345678"
run_test 2  "Small Input Test"       1  "testkey12345678"
run_test 3  "Medium Input Test"      32 "testkey12345678"
run_test 4  "Large Input Test"       64 "testkey12345678"
run_test 5  "Different Key Test 1"   16 "key1234567890123"
run_test 6  "Different Key Test 2"   16 "differentkey1234"
run_test 7  "Maximum Input Test"     64 "testkey12345678"
run_test 8  "Binary Input Test"      16 "binary_key_test1"
run_test 9  "Special Chars Test"     16 "special@key#123"
run_test 10 "Numeric Key Test"       16 "123456789012345"
run_test 11 "Boundary Test 1"        63 "testkey12345678"
run_test 12 "Boundary Test 2"        2  "testkey12345678"
run_test 13 "Mixed Input Test"       48 "mixedkey123test"
run_test 14 "Sequential Input"       16 "sequential_key_1"
run_test 15 "Unicode Key Test"       16 "unicode_key_テスト"
run_test 16 "Long Input Test"        64 "long_key_test123"
run_test 17 "Short Input Test"       4  "short_key_test1"
run_test 18 "Random Input Test 1"    24 "random_key_test1"
run_test 19 "Random Input Test 2"    40 "random_key_test2"
run_test 20 "Edge Case Test"         64 "edge_case_key_12"

exit $failed

