
#!/bin/bash

# Reformed test cases

mkdir -p flow_results
failed=0

run_testcase() {
    local test_num=$1
    local binary=$2
    local expected=$3

    echo "Test ${test_num} started"
    local log_file="flow_results/test${test_num}_tmp.log"
    : > "$log_file"

    local actual
    actual=$(LD_PRELOAD=libtracer.so TRACE_OUTPUT=$PWD/flow_results/test${test_num}_trace.log ./${binary} ${test_num} 2>&1)

    {
        echo "Running test ${test_num}..."
        echo "----------------------------------------"
        echo "Expected length: ${#expected}"
        echo "Actual length: ${#actual}"
        echo "----------------------------------------"
    } >> "$log_file"

    if [ "$expected" = "$actual" ]; then
        {
            echo "✓ Test ${test_num} PASSED - Output matches expected result"
        } >> "$log_file"
        mv "$log_file" "flow_results/test${test_num}_success.log"
        echo "Test ${test_num} passed"
    else
        {
            echo "✗ Test ${test_num} FAILED - Output differs from expected result"
            echo ""
            echo "=== EXPECTED OUTPUT ==="
            echo "$expected"
            echo ""
            echo "=== ACTUAL OUTPUT ==="
            echo "$actual"
            echo ""
            echo "=== DETAILED COMPARISON ==="
            echo "Hex dump of expected (first 200 bytes):"
            echo "$expected" | xxd | head -20
            echo ""
            echo "Hex dump of actual (first 200 bytes):"
            echo "$actual" | xxd | head -20
            echo ""
            echo "=== DIFF ==="
            diff <(echo "$expected") <(echo "$actual")
        } >> "$log_file"
        mv "$log_file" "flow_results/test${test_num}_fail.log"
        echo "Test ${test_num} failed" >&2
        failed=1
    fi
    echo "Test ${test_num} ended"
}

# Test 1: Named colors
EXPECTED1="Running test 1: Named colors
Value: 1
Parse status: OK
Expected: -2139094785, Actual: -2139094785 - PASS"
run_testcase 1 test_t1 "$EXPECTED1"

# Test 2: RGB format
EXPECTED2="Running test 2: RGB format
Value: 1
Parse status: OK
Expected: -14810881, Actual: -14810881 - PASS
Value: 1
Parse status: OK
Expected: 255, Actual: 255 - PASS"
run_testcase 2 test_t2 "$EXPECTED2"

# Test 3: RGBA format
EXPECTED3="Running test 3: RGBA format
Value: 1
Parse status: OK
Expected: -14811009, Actual: -14811009 - PASS
Value: 1
Parse status: OK
Expected: 255, Actual: 255 - PASS"
run_testcase 3 test_t3 "$EXPECTED3"

# Test 4: Hex format
EXPECTED4="Running test 4: Hex format
Value: 1
Parse status: OK
Expected: -14810881, Actual: -14810881 - PASS
Value: 1
Parse status: OK
Expected: -1, Actual: -1 - PASS
Value: 1
Parse status: OK
Expected: -3407617, Actual: -3407617 - PASS
Value: 1
Parse status: OK
Expected: -3407617, Actual: -3407617 - PASS"
run_testcase 4 test_t4 "$EXPECTED4"

# Test 5: To string conversion
EXPECTED5="Running test 5: To string conversion
Expected: 0, Actual: 0 - PASS
Expected: 0, Actual: 0 - PASS"
run_testcase 5 test_t5 "$EXPECTED5"

echo "All tests completed."
exit $failed

