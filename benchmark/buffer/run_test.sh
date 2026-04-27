
#!/bin/bash

# Reformed test cases

failed=0
mkdir -p flow_results

EXPECTED_DIR="expected"

run_test() {
    local test_num=$1
    local binary="./test_t${test_num}"
    local expected_file="${EXPECTED_DIR}/expected_values_${test_num}.txt"
    local success_log="flow_results/test${test_num}_success.log"
    local fail_log="flow_results/test${test_num}_fail.log"
    local tmp_log="flow_results/test${test_num}_tmp.log"

    echo "Test ${test_num} started"

    if [ ! -x "$binary" ]; then
        echo "Test ${test_num} failed: binary ${binary} not found" >&2
        echo "Binary ${binary} not found" > "$fail_log"
        echo "Test ${test_num} failed" >&2
        echo "Test ${test_num} ended"
        failed=1
        return
    fi

    LD_PRELOAD=libtracer.so TRACE_OUTPUT=$PWD/flow_results/test${test_num}_trace.log "$binary" "$test_num" > "$tmp_log" 2>&1
    local rc=$?

    if [ $rc -ne 0 ]; then
        echo "Binary exited with status ${rc}" >> "$tmp_log"
        mv "$tmp_log" "$fail_log"
        echo "Test ${test_num} failed" >&2
        echo "Test ${test_num} ended"
        failed=1
        return
    fi

    if [ ! -f "$expected_file" ]; then
        echo "Expected file ${expected_file} not found" >> "$tmp_log"
        mv "$tmp_log" "$fail_log"
        echo "Test ${test_num} failed" >&2
        echo "Test ${test_num} ended"
        failed=1
        return
    fi

    local actual_output
    actual_output=$(cat "$tmp_log")
    local expected_output
    expected_output=$(cat "$expected_file")

    if [ "$actual_output" = "$expected_output" ]; then
        mv "$tmp_log" "$success_log"
        echo "Test ${test_num} passed"
    else
        {
            echo "=== EXPECTED ==="
            echo "$expected_output"
            echo "=== ACTUAL ==="
            echo "$actual_output"
            echo "=== DIFF ==="
            diff <(echo "$expected_output") <(echo "$actual_output")
        } >> "$tmp_log"
        mv "$tmp_log" "$fail_log"
        echo "Test ${test_num} failed" >&2
        failed=1
    fi

    echo "Test ${test_num} ended"
}

for i in 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16 17; do
    run_test "$i"
done

exit $failed

