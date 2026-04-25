
#!/bin/bash

# Reformed test cases

failed=0
mkdir -p flow_results

# Test 1: Named colors
echo "Test 1 started"
expected1="test_named passed"
actual1=$(LD_PRELOAD=libtracer.so TRACE_OUTPUT=$PWD/flow_results/test1_trace.log ./test_rgba_t1 1 2>&1)
rc1=$?
if [ $rc1 -eq 0 ] && echo "$actual1" | grep -q "$expected1"; then
    echo "$actual1" > flow_results/test1_success.log
    echo "Test 1 passed"
else
    {
        echo "Exit code: $rc1"
        echo "=== Expected to contain ==="
        echo "$expected1"
        echo "=== Actual ==="
        echo "$actual1"
    } > flow_results/test1_fail.log
    echo "Test 1 failed" >&2
    failed=1
fi
echo "Test 1 ended"

# Test 2: RGB format
echo "Test 2 started"
expected2="test_rgb passed"
actual2=$(LD_PRELOAD=libtracer.so TRACE_OUTPUT=$PWD/flow_results/test2_trace.log ./test_rgba_t2 2 2>&1)
rc2=$?
if [ $rc2 -eq 0 ] && echo "$actual2" | grep -q "$expected2"; then
    echo "$actual2" > flow_results/test2_success.log
    echo "Test 2 passed"
else
    {
        echo "Exit code: $rc2"
        echo "=== Expected to contain ==="
        echo "$expected2"
        echo "=== Actual ==="
        echo "$actual2"
    } > flow_results/test2_fail.log
    echo "Test 2 failed" >&2
    failed=1
fi
echo "Test 2 ended"

# Test 3: RGBA format
echo "Test 3 started"
expected3="test_rgba passed"
actual3=$(LD_PRELOAD=libtracer.so TRACE_OUTPUT=$PWD/flow_results/test3_trace.log ./test_rgba_t3 3 2>&1)
rc3=$?
if [ $rc3 -eq 0 ] && echo "$actual3" | grep -q "$expected3"; then
    echo "$actual3" > flow_results/test3_success.log
    echo "Test 3 passed"
else
    {
        echo "Exit code: $rc3"
        echo "=== Expected to contain ==="
        echo "$expected3"
        echo "=== Actual ==="
        echo "$actual3"
    } > flow_results/test3_fail.log
    echo "Test 3 failed" >&2
    failed=1
fi
echo "Test 3 ended"

# Test 4: Hex format
echo "Test 4 started"
expected4="test_hex passed"
actual4=$(LD_PRELOAD=libtracer.so TRACE_OUTPUT=$PWD/flow_results/test4_trace.log ./test_rgba_t4 4 2>&1)
rc4=$?
if [ $rc4 -eq 0 ] && echo "$actual4" | grep -q "$expected4"; then
    echo "$actual4" > flow_results/test4_success.log
    echo "Test 4 passed"
else
    {
        echo "Exit code: $rc4"
        echo "=== Expected to contain ==="
        echo "$expected4"
        echo "=== Actual ==="
        echo "$actual4"
    } > flow_results/test4_fail.log
    echo "Test 4 failed" >&2
    failed=1
fi
echo "Test 4 ended"

# Test 5: To string conversion
echo "Test 5 started"
expected5="test_to_string passed"
actual5=$(LD_PRELOAD=libtracer.so TRACE_OUTPUT=$PWD/flow_results/test5_trace.log ./test_rgba_t5 5 2>&1)
rc5=$?
if [ $rc5 -eq 0 ] && echo "$actual5" | grep -q "$expected5"; then
    echo "$actual5" > flow_results/test5_success.log
    echo "Test 5 passed"
else
    {
        echo "Exit code: $rc5"
        echo "=== Expected to contain ==="
        echo "$expected5"
        echo "=== Actual ==="
        echo "$actual5"
    } > flow_results/test5_fail.log
    echo "Test 5 failed" >&2
    failed=1
fi
echo "Test 5 ended"

exit $failed

