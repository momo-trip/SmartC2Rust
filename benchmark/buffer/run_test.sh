
#!/bin/bash

# Reformed test cases

failed=0

RESULTS_DIR="flow_results"
mkdir -p "$RESULTS_DIR"

EXPECTED_DIR="expected"

run_testcase() {
    local test_num=$1
    local binary="./test_t${test_num}"
    local log_file="$RESULTS_DIR/test${test_num}_tmp.log"
    local expected_file="$EXPECTED_DIR/expected_values_${test_num}.txt"

    echo "Test ${test_num} started"

    {
        echo "Running buffer test ${test_num}..."

        if [ ! -x "$binary" ]; then
            echo "Binary $binary not found or not executable"
            echo "Test ${test_num} ended"
            echo "__RESULT__=FAIL"
            exit 0
        fi

        TEST_OUTPUT=$(LD_PRELOAD=libtracer.so TRACE_OUTPUT=$PWD/$RESULTS_DIR/test${test_num}_trace.log "$binary" ${test_num} 2>&1)
        echo "$TEST_OUTPUT"

        if [ ! -f "$expected_file" ]; then
            echo "Expected values file not found: $expected_file"
            echo "__RESULT__=FAIL"
        else
            expected_output=$(cat "$expected_file")
            if [ "$TEST_OUTPUT" = "$expected_output" ]; then
                echo "Output matches expected values"
                echo "__RESULT__=PASS"
            else
                echo "Output differs from expected values"
                echo "Expected:"
                echo "$expected_output"
                echo "Actual:"
                echo "$TEST_OUTPUT"
                echo "__RESULT__=FAIL"
            fi
        fi
    } > "$log_file" 2>&1

    if grep -q "__RESULT__=PASS" "$log_file"; then
        mv "$log_file" "$RESULTS_DIR/test${test_num}_success.log"
        echo "Test ${test_num} ended"
        echo "Test ${test_num} passed"
    else
        mv "$log_file" "$RESULTS_DIR/test${test_num}_fail.log"
        echo "Test ${test_num} ended"
        echo "Test ${test_num} failed" >&2
        failed=1
    fi
}

for i in 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16 17; do
    run_testcase $i
done

exit $failed

