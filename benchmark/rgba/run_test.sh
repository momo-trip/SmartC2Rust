
#!/bin/bash

# Reformed test cases

failed=0
mkdir -p flow_results

# ---------------- Test 1: Named colors ----------------
test_num=1
echo "Test ${test_num} started"
EXPECTED_OUTPUT="Running test 1: Named colors
Value: 1
Parse status: OK
Expected: -2139094785, Actual: -2139094785 - PASS"
ACTUAL_OUTPUT=$(LD_PRELOAD=libtracer.so TRACE_OUTPUT=$PWD/flow_results/test${test_num}_trace.log ./test_t1 ${test_num} 2>&1)
rc=$?
if [ $rc -eq 0 ] && [ "$EXPECTED_OUTPUT" = "$ACTUAL_OUTPUT" ]; then
    echo "Test ${test_num} passed"
    {
        echo "=== EXPECTED ==="
        echo "$EXPECTED_OUTPUT"
        echo "=== ACTUAL ==="
        echo "$ACTUAL_OUTPUT"
    } > flow_results/test${test_num}_success.log
else
    echo "Test ${test_num} failed" >&2
    failed=1
    {
        echo "Return code: $rc"
        echo "=== EXPECTED ==="
        echo "$EXPECTED_OUTPUT"
        echo "=== ACTUAL ==="
        echo "$ACTUAL_OUTPUT"
    } > flow_results/test${test_num}_fail.log
fi
echo "Test ${test_num} ended"

# ---------------- Test 2: RGB format ----------------
test_num=2
echo "Test ${test_num} started"
EXPECTED_OUTPUT="Running test 2: RGB format
Value: 1
Parse status: OK
Expected: -14810881, Actual: -14810881 - PASS
Value: 1
Parse status: OK
Expected: 255, Actual: 255 - PASS"
ACTUAL_OUTPUT=$(LD_PRELOAD=libtracer.so TRACE_OUTPUT=$PWD/flow_results/test${test_num}_trace.log ./test_t2 ${test_num} 2>&1)
rc=$?
if [ $rc -eq 0 ] && [ "$EXPECTED_OUTPUT" = "$ACTUAL_OUTPUT" ]; then
    echo "Test ${test_num} passed"
    {
        echo "=== EXPECTED ==="
        echo "$EXPECTED_OUTPUT"
        echo "=== ACTUAL ==="
        echo "$ACTUAL_OUTPUT"
    } > flow_results/test${test_num}_success.log
else
    echo "Test ${test_num} failed" >&2
    failed=1
    {
        echo "Return code: $rc"
        echo "=== EXPECTED ==="
        echo "$EXPECTED_OUTPUT"
        echo "=== ACTUAL ==="
        echo "$ACTUAL_OUTPUT"
    } > flow_results/test${test_num}_fail.log
fi
echo "Test ${test_num} ended"

# ---------------- Test 3: RGBA format ----------------
test_num=3
echo "Test ${test_num} started"
EXPECTED_OUTPUT="Running test 3: RGBA format
Value: 1
Parse status: OK
Expected: -14811009, Actual: -14811009 - PASS
Value: 1
Parse status: OK
Expected: 255, Actual: 255 - PASS"
ACTUAL_OUTPUT=$(LD_PRELOAD=libtracer.so TRACE_OUTPUT=$PWD/flow_results/test${test_num}_trace.log ./test_t3 ${test_num} 2>&1)
rc=$?
if [ $rc -eq 0 ] && [ "$EXPECTED_OUTPUT" = "$ACTUAL_OUTPUT" ]; then
    echo "Test ${test_num} passed"
    {
        echo "=== EXPECTED ==="
        echo "$EXPECTED_OUTPUT"
        echo "=== ACTUAL ==="
        echo "$ACTUAL_OUTPUT"
    } > flow_results/test${test_num}_success.log
else
    echo "Test ${test_num} failed" >&2
    failed=1
    {
        echo "Return code: $rc"
        echo "=== EXPECTED ==="
        echo "$EXPECTED_OUTPUT"
        echo "=== ACTUAL ==="
        echo "$ACTUAL_OUTPUT"
    } > flow_results/test${test_num}_fail.log
fi
echo "Test ${test_num} ended"

# ---------------- Test 4: Hex format ----------------
test_num=4
echo "Test ${test_num} started"
EXPECTED_OUTPUT="Running test 4: Hex format
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
ACTUAL_OUTPUT=$(LD_PRELOAD=libtracer.so TRACE_OUTPUT=$PWD/flow_results/test${test_num}_trace.log ./test_t4 ${test_num} 2>&1)
rc=$?
if [ $rc -eq 0 ] && [ "$EXPECTED_OUTPUT" = "$ACTUAL_OUTPUT" ]; then
    echo "Test ${test_num} passed"
    {
        echo "=== EXPECTED ==="
        echo "$EXPECTED_OUTPUT"
        echo "=== ACTUAL ==="
        echo "$ACTUAL_OUTPUT"
    } > flow_results/test${test_num}_success.log
else
    echo "Test ${test_num} failed" >&2
    failed=1
    {
        echo "Return code: $rc"
        echo "=== EXPECTED ==="
        echo "$EXPECTED_OUTPUT"
        echo "=== ACTUAL ==="
        echo "$ACTUAL_OUTPUT"
    } > flow_results/test${test_num}_fail.log
fi
echo "Test ${test_num} ended"

# ---------------- Test 5: To string conversion ----------------
test_num=5
echo "Test ${test_num} started"
EXPECTED_OUTPUT="Running test 5: To string conversion
Expected: 0, Actual: 0 - PASS
Expected: 0, Actual: 0 - PASS"
ACTUAL_OUTPUT=$(LD_PRELOAD=libtracer.so TRACE_OUTPUT=$PWD/flow_results/test${test_num}_trace.log ./test_t5 ${test_num} 2>&1)
rc=$?
if [ $rc -eq 0 ] && [ "$EXPECTED_OUTPUT" = "$ACTUAL_OUTPUT" ]; then
    echo "Test ${test_num} passed"
    {
        echo "=== EXPECTED ==="
        echo "$EXPECTED_OUTPUT"
        echo "=== ACTUAL ==="
        echo "$ACTUAL_OUTPUT"
    } > flow_results/test${test_num}_success.log
else
    echo "Test ${test_num} failed" >&2
    failed=1
    {
        echo "Return code: $rc"
        echo "=== EXPECTED ==="
        echo "$EXPECTED_OUTPUT"
        echo "=== ACTUAL ==="
        echo "$ACTUAL_OUTPUT"
    } > flow_results/test${test_num}_fail.log
fi
echo "Test ${test_num} ended"

exit $failed

