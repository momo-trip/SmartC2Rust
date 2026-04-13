
#!/bin/bash

# Reformed test cases

failed=0

# Expected values directory
EXPECTED_DIR="expected"

# Create flow_results directory
mkdir -p flow_results

# Test 1
echo "Test 1 started"
TEST_OUTPUT=$(LD_PRELOAD=libtracer.so TRACE_OUTPUT=$PWD/flow_results/test1_trace.log ./test_t1 1 2>&1)
TEST_RESULT=$?
EXPECTED_OUTPUT=$(cat "$EXPECTED_DIR/expected_values_1.txt")
if [ $TEST_RESULT -eq 0 ] && [ "$TEST_OUTPUT" = "$EXPECTED_OUTPUT" ]; then
    echo "Test 1 passed"
    echo "$TEST_OUTPUT" > flow_results/test1_success.log
else
    echo "Test 1 failed"
    echo "Test 1 failed" >&2
    echo "$TEST_OUTPUT" > flow_results/test1_fail.log
    failed=1
fi
echo "Test 1 ended"

# Test 2
echo "Test 2 started"
TEST_OUTPUT=$(LD_PRELOAD=libtracer.so TRACE_OUTPUT=$PWD/flow_results/test2_trace.log ./test_t2 2 2>&1)
TEST_RESULT=$?
EXPECTED_OUTPUT=$(cat "$EXPECTED_DIR/expected_values_2.txt")
if [ $TEST_RESULT -eq 0 ] && [ "$TEST_OUTPUT" = "$EXPECTED_OUTPUT" ]; then
    echo "Test 2 passed"
    echo "$TEST_OUTPUT" > flow_results/test2_success.log
else
    echo "Test 2 failed"
    echo "Test 2 failed" >&2
    echo "$TEST_OUTPUT" > flow_results/test2_fail.log
    failed=1
fi
echo "Test 2 ended"

# Test 3
echo "Test 3 started"
TEST_OUTPUT=$(LD_PRELOAD=libtracer.so TRACE_OUTPUT=$PWD/flow_results/test3_trace.log ./test_t3 3 2>&1)
TEST_RESULT=$?
EXPECTED_OUTPUT=$(cat "$EXPECTED_DIR/expected_values_3.txt")
if [ $TEST_RESULT -eq 0 ] && [ "$TEST_OUTPUT" = "$EXPECTED_OUTPUT" ]; then
    echo "Test 3 passed"
    echo "$TEST_OUTPUT" > flow_results/test3_success.log
else
    echo "Test 3 failed"
    echo "Test 3 failed" >&2
    echo "$TEST_OUTPUT" > flow_results/test3_fail.log
    failed=1
fi
echo "Test 3 ended"

# Test 4
echo "Test 4 started"
TEST_OUTPUT=$(LD_PRELOAD=libtracer.so TRACE_OUTPUT=$PWD/flow_results/test4_trace.log ./test_t4 4 2>&1)
TEST_RESULT=$?
EXPECTED_OUTPUT=$(cat "$EXPECTED_DIR/expected_values_4.txt")
if [ $TEST_RESULT -eq 0 ] && [ "$TEST_OUTPUT" = "$EXPECTED_OUTPUT" ]; then
    echo "Test 4 passed"
    echo "$TEST_OUTPUT" > flow_results/test4_success.log
else
    echo "Test 4 failed"
    echo "Test 4 failed" >&2
    echo "$TEST_OUTPUT" > flow_results/test4_fail.log
    failed=1
fi
echo "Test 4 ended"

# Test 5
echo "Test 5 started"
TEST_OUTPUT=$(LD_PRELOAD=libtracer.so TRACE_OUTPUT=$PWD/flow_results/test5_trace.log ./test_t5 5 2>&1)
TEST_RESULT=$?
EXPECTED_OUTPUT=$(cat "$EXPECTED_DIR/expected_values_5.txt")
if [ $TEST_RESULT -eq 0 ] && [ "$TEST_OUTPUT" = "$EXPECTED_OUTPUT" ]; then
    echo "Test 5 passed"
    echo "$TEST_OUTPUT" > flow_results/test5_success.log
else
    echo "Test 5 failed"
    echo "Test 5 failed" >&2
    echo "$TEST_OUTPUT" > flow_results/test5_fail.log
    failed=1
fi
echo "Test 5 ended"

# Test 6
echo "Test 6 started"
TEST_OUTPUT=$(LD_PRELOAD=libtracer.so TRACE_OUTPUT=$PWD/flow_results/test6_trace.log ./test_t6 6 2>&1)
TEST_RESULT=$?
EXPECTED_OUTPUT=$(cat "$EXPECTED_DIR/expected_values_6.txt")
if [ $TEST_RESULT -eq 0 ] && [ "$TEST_OUTPUT" = "$EXPECTED_OUTPUT" ]; then
    echo "Test 6 passed"
    echo "$TEST_OUTPUT" > flow_results/test6_success.log
else
    echo "Test 6 failed"
    echo "Test 6 failed" >&2
    echo "$TEST_OUTPUT" > flow_results/test6_fail.log
    failed=1
fi
echo "Test 6 ended"

# Test 7
echo "Test 7 started"
TEST_OUTPUT=$(LD_PRELOAD=libtracer.so TRACE_OUTPUT=$PWD/flow_results/test7_trace.log ./test_t7 7 2>&1)
TEST_RESULT=$?
EXPECTED_OUTPUT=$(cat "$EXPECTED_DIR/expected_values_7.txt")
if [ $TEST_RESULT -eq 0 ] && [ "$TEST_OUTPUT" = "$EXPECTED_OUTPUT" ]; then
    echo "Test 7 passed"
    echo "$TEST_OUTPUT" > flow_results/test7_success.log
else
    echo "Test 7 failed"
    echo "Test 7 failed" >&2
    echo "$TEST_OUTPUT" > flow_results/test7_fail.log
    failed=1
fi
echo "Test 7 ended"

# Test 8
echo "Test 8 started"
TEST_OUTPUT=$(LD_PRELOAD=libtracer.so TRACE_OUTPUT=$PWD/flow_results/test8_trace.log ./test_t8 8 2>&1)
TEST_RESULT=$?
EXPECTED_OUTPUT=$(cat "$EXPECTED_DIR/expected_values_8.txt")
if [ $TEST_RESULT -eq 0 ] && [ "$TEST_OUTPUT" = "$EXPECTED_OUTPUT" ]; then
    echo "Test 8 passed"
    echo "$TEST_OUTPUT" > flow_results/test8_success.log
else
    echo "Test 8 failed"
    echo "Test 8 failed" >&2
    echo "$TEST_OUTPUT" > flow_results/test8_fail.log
    failed=1
fi
echo "Test 8 ended"

# Test 9
echo "Test 9 started"
TEST_OUTPUT=$(LD_PRELOAD=libtracer.so TRACE_OUTPUT=$PWD/flow_results/test9_trace.log ./test_t9 9 2>&1)
TEST_RESULT=$?
EXPECTED_OUTPUT=$(cat "$EXPECTED_DIR/expected_values_9.txt")
if [ $TEST_RESULT -eq 0 ] && [ "$TEST_OUTPUT" = "$EXPECTED_OUTPUT" ]; then
    echo "Test 9 passed"
    echo "$TEST_OUTPUT" > flow_results/test9_success.log
else
    echo "Test 9 failed"
    echo "Test 9 failed" >&2
    echo "$TEST_OUTPUT" > flow_results/test9_fail.log
    failed=1
fi
echo "Test 9 ended"

# Test 10
echo "Test 10 started"
TEST_OUTPUT=$(LD_PRELOAD=libtracer.so TRACE_OUTPUT=$PWD/flow_results/test10_trace.log ./test_t10 10 2>&1)
TEST_RESULT=$?
EXPECTED_OUTPUT=$(cat "$EXPECTED_DIR/expected_values_10.txt")
if [ $TEST_RESULT -eq 0 ] && [ "$TEST_OUTPUT" = "$EXPECTED_OUTPUT" ]; then
    echo "Test 10 passed"
    echo "$TEST_OUTPUT" > flow_results/test10_success.log
else
    echo "Test 10 failed"
    echo "Test 10 failed" >&2
    echo "$TEST_OUTPUT" > flow_results/test10_fail.log
    failed=1
fi
echo "Test 10 ended"

# Test 11
echo "Test 11 started"
TEST_OUTPUT=$(LD_PRELOAD=libtracer.so TRACE_OUTPUT=$PWD/flow_results/test11_trace.log ./test_t11 11 2>&1)
TEST_RESULT=$?
EXPECTED_OUTPUT=$(cat "$EXPECTED_DIR/expected_values_11.txt")
if [ $TEST_RESULT -eq 0 ] && [ "$TEST_OUTPUT" = "$EXPECTED_OUTPUT" ]; then
    echo "Test 11 passed"
    echo "$TEST_OUTPUT" > flow_results/test11_success.log
else
    echo "Test 11 failed"
    echo "Test 11 failed" >&2
    echo "$TEST_OUTPUT" > flow_results/test11_fail.log
    failed=1
fi
echo "Test 11 ended"

# Test 12
echo "Test 12 started"
TEST_OUTPUT=$(LD_PRELOAD=libtracer.so TRACE_OUTPUT=$PWD/flow_results/test12_trace.log ./test_t12 12 2>&1)
TEST_RESULT=$?
EXPECTED_OUTPUT=$(cat "$EXPECTED_DIR/expected_values_12.txt")
if [ $TEST_RESULT -eq 0 ] && [ "$TEST_OUTPUT" = "$EXPECTED_OUTPUT" ]; then
    echo "Test 12 passed"
    echo "$TEST_OUTPUT" > flow_results/test12_success.log
else
    echo "Test 12 failed"
    echo "Test 12 failed" >&2
    echo "$TEST_OUTPUT" > flow_results/test12_fail.log
    failed=1
fi
echo "Test 12 ended"

# Test 13
echo "Test 13 started"
TEST_OUTPUT=$(LD_PRELOAD=libtracer.so TRACE_OUTPUT=$PWD/flow_results/test13_trace.log ./test_t13 13 2>&1)
TEST_RESULT=$?
EXPECTED_OUTPUT=$(cat "$EXPECTED_DIR/expected_values_13.txt")
if [ $TEST_RESULT -eq 0 ] && [ "$TEST_OUTPUT" = "$EXPECTED_OUTPUT" ]; then
    echo "Test 13 passed"
    echo "$TEST_OUTPUT" > flow_results/test13_success.log
else
    echo "Test 13 failed"
    echo "Test 13 failed" >&2
    echo "$TEST_OUTPUT" > flow_results/test13_fail.log
    failed=1
fi
echo "Test 13 ended"

# Test 14
echo "Test 14 started"
TEST_OUTPUT=$(LD_PRELOAD=libtracer.so TRACE_OUTPUT=$PWD/flow_results/test14_trace.log ./test_t14 14 2>&1)
TEST_RESULT=$?
EXPECTED_OUTPUT=$(cat "$EXPECTED_DIR/expected_values_14.txt")
if [ $TEST_RESULT -eq 0 ] && [ "$TEST_OUTPUT" = "$EXPECTED_OUTPUT" ]; then
    echo "Test 14 passed"
    echo "$TEST_OUTPUT" > flow_results/test14_success.log
else
    echo "Test 14 failed"
    echo "Test 14 failed" >&2
    echo "$TEST_OUTPUT" > flow_results/test14_fail.log
    failed=1
fi
echo "Test 14 ended"

# Test 15
echo "Test 15 started"
TEST_OUTPUT=$(LD_PRELOAD=libtracer.so TRACE_OUTPUT=$PWD/flow_results/test15_trace.log ./test_t15 15 2>&1)
TEST_RESULT=$?
EXPECTED_OUTPUT=$(cat "$EXPECTED_DIR/expected_values_15.txt")
if [ $TEST_RESULT -eq 0 ] && [ "$TEST_OUTPUT" = "$EXPECTED_OUTPUT" ]; then
    echo "Test 15 passed"
    echo "$TEST_OUTPUT" > flow_results/test15_success.log
else
    echo "Test 15 failed"
    echo "Test 15 failed" >&2
    echo "$TEST_OUTPUT" > flow_results/test15_fail.log
    failed=1
fi
echo "Test 15 ended"

# Test 16
echo "Test 16 started"
TEST_OUTPUT=$(LD_PRELOAD=libtracer.so TRACE_OUTPUT=$PWD/flow_results/test16_trace.log ./test_t16 16 2>&1)
TEST_RESULT=$?
EXPECTED_OUTPUT=$(cat "$EXPECTED_DIR/expected_values_16.txt")
if [ $TEST_RESULT -eq 0 ] && [ "$TEST_OUTPUT" = "$EXPECTED_OUTPUT" ]; then
    echo "Test 16 passed"
    echo "$TEST_OUTPUT" > flow_results/test16_success.log
else
    echo "Test 16 failed"
    echo "Test 16 failed" >&2
    echo "$TEST_OUTPUT" > flow_results/test16_fail.log
    failed=1
fi
echo "Test 16 ended"

# Test 17
echo "Test 17 started"
TEST_OUTPUT=$(LD_PRELOAD=libtracer.so TRACE_OUTPUT=$PWD/flow_results/test17_trace.log ./test_t17 17 2>&1)
TEST_RESULT=$?
EXPECTED_OUTPUT=$(cat "$EXPECTED_DIR/expected_values_17.txt")
if [ $TEST_RESULT -eq 0 ] && [ "$TEST_OUTPUT" = "$EXPECTED_OUTPUT" ]; then
    echo "Test 17 passed"
    echo "$TEST_OUTPUT" > flow_results/test17_success.log
else
    echo "Test 17 failed"
    echo "Test 17 failed" >&2
    echo "$TEST_OUTPUT" > flow_results/test17_fail.log
    failed=1
fi
echo "Test 17 ended"

exit $failed

