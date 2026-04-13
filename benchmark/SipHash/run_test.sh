
#!/bin/bash

# Reformed test cases
failed=0

mkdir -p flow_results

# Generate expected output once (all tests use the same deterministic binary)
expected_output=$(./vectors_t1 2>&1)

# Test 1: Empty Input Test
echo "Test 1 started"
actual_output=$(LD_PRELOAD=libtracer.so TRACE_OUTPUT=$PWD/flow_results/test1_trace.log ./vectors_t1 2>&1)
test1_status=$?
if [ $test1_status -eq 0 ] && [ "$actual_output" = "$expected_output" ]; then
    echo "Test 1 passed"
    echo "Test: Empty Input Test" > flow_results/test1_success.log
    echo "Actual output:" >> flow_results/test1_success.log
    echo "$actual_output" >> flow_results/test1_success.log
else
    echo "Test 1 failed"
    echo "Test 1 failed" >&2
    echo "Test: Empty Input Test" > flow_results/test1_fail.log
    echo "Expected output:" >> flow_results/test1_fail.log
    echo "$expected_output" >> flow_results/test1_fail.log
    echo "Actual output:" >> flow_results/test1_fail.log
    echo "$actual_output" >> flow_results/test1_fail.log
    failed=1
fi
echo "Test 1 ended"

# Test 2: Small Input Test
echo "Test 2 started"
actual_output=$(LD_PRELOAD=libtracer.so TRACE_OUTPUT=$PWD/flow_results/test2_trace.log ./vectors_t2 2>&1)
test2_status=$?
if [ $test2_status -eq 0 ] && [ "$actual_output" = "$expected_output" ]; then
    echo "Test 2 passed"
    echo "Test: Small Input Test" > flow_results/test2_success.log
    echo "Actual output:" >> flow_results/test2_success.log
    echo "$actual_output" >> flow_results/test2_success.log
else
    echo "Test 2 failed"
    echo "Test 2 failed" >&2
    echo "Test: Small Input Test" > flow_results/test2_fail.log
    echo "Expected output:" >> flow_results/test2_fail.log
    echo "$expected_output" >> flow_results/test2_fail.log
    echo "Actual output:" >> flow_results/test2_fail.log
    echo "$actual_output" >> flow_results/test2_fail.log
    failed=1
fi
echo "Test 2 ended"

# Test 3: Medium Input Test
echo "Test 3 started"
actual_output=$(LD_PRELOAD=libtracer.so TRACE_OUTPUT=$PWD/flow_results/test3_trace.log ./vectors_t3 2>&1)
test3_status=$?
if [ $test3_status -eq 0 ] && [ "$actual_output" = "$expected_output" ]; then
    echo "Test 3 passed"
    echo "Test: Medium Input Test" > flow_results/test3_success.log
    echo "Actual output:" >> flow_results/test3_success.log
    echo "$actual_output" >> flow_results/test3_success.log
else
    echo "Test 3 failed"
    echo "Test 3 failed" >&2
    echo "Test: Medium Input Test" > flow_results/test3_fail.log
    echo "Expected output:" >> flow_results/test3_fail.log
    echo "$expected_output" >> flow_results/test3_fail.log
    echo "Actual output:" >> flow_results/test3_fail.log
    echo "$actual_output" >> flow_results/test3_fail.log
    failed=1
fi
echo "Test 3 ended"

# Test 4: Large Input Test
echo "Test 4 started"
actual_output=$(LD_PRELOAD=libtracer.so TRACE_OUTPUT=$PWD/flow_results/test4_trace.log ./vectors_t4 2>&1)
test4_status=$?
if [ $test4_status -eq 0 ] && [ "$actual_output" = "$expected_output" ]; then
    echo "Test 4 passed"
    echo "Test: Large Input Test" > flow_results/test4_success.log
    echo "Actual output:" >> flow_results/test4_success.log
    echo "$actual_output" >> flow_results/test4_success.log
else
    echo "Test 4 failed"
    echo "Test 4 failed" >&2
    echo "Test: Large Input Test" > flow_results/test4_fail.log
    echo "Expected output:" >> flow_results/test4_fail.log
    echo "$expected_output" >> flow_results/test4_fail.log
    echo "Actual output:" >> flow_results/test4_fail.log
    echo "$actual_output" >> flow_results/test4_fail.log
    failed=1
fi
echo "Test 4 ended"

# Test 5: Different Key Test 1
echo "Test 5 started"
actual_output=$(LD_PRELOAD=libtracer.so TRACE_OUTPUT=$PWD/flow_results/test5_trace.log ./vectors_t5 2>&1)
test5_status=$?
if [ $test5_status -eq 0 ] && [ "$actual_output" = "$expected_output" ]; then
    echo "Test 5 passed"
    echo "Test: Different Key Test 1" > flow_results/test5_success.log
    echo "Actual output:" >> flow_results/test5_success.log
    echo "$actual_output" >> flow_results/test5_success.log
else
    echo "Test 5 failed"
    echo "Test 5 failed" >&2
    echo "Test: Different Key Test 1" > flow_results/test5_fail.log
    echo "Expected output:" >> flow_results/test5_fail.log
    echo "$expected_output" >> flow_results/test5_fail.log
    echo "Actual output:" >> flow_results/test5_fail.log
    echo "$actual_output" >> flow_results/test5_fail.log
    failed=1
fi
echo "Test 5 ended"

# Test 6: Different Key Test 2
echo "Test 6 started"
actual_output=$(LD_PRELOAD=libtracer.so TRACE_OUTPUT=$PWD/flow_results/test6_trace.log ./vectors_t6 2>&1)
test6_status=$?
if [ $test6_status -eq 0 ] && [ "$actual_output" = "$expected_output" ]; then
    echo "Test 6 passed"
    echo "Test: Different Key Test 2" > flow_results/test6_success.log
    echo "Actual output:" >> flow_results/test6_success.log
    echo "$actual_output" >> flow_results/test6_success.log
else
    echo "Test 6 failed"
    echo "Test 6 failed" >&2
    echo "Test: Different Key Test 2" > flow_results/test6_fail.log
    echo "Expected output:" >> flow_results/test6_fail.log
    echo "$expected_output" >> flow_results/test6_fail.log
    echo "Actual output:" >> flow_results/test6_fail.log
    echo "$actual_output" >> flow_results/test6_fail.log
    failed=1
fi
echo "Test 6 ended"

# Test 7: Maximum Input Test
echo "Test 7 started"
actual_output=$(LD_PRELOAD=libtracer.so TRACE_OUTPUT=$PWD/flow_results/test7_trace.log ./vectors_t7 2>&1)
test7_status=$?
if [ $test7_status -eq 0 ] && [ "$actual_output" = "$expected_output" ]; then
    echo "Test 7 passed"
    echo "Test: Maximum Input Test" > flow_results/test7_success.log
    echo "Actual output:" >> flow_results/test7_success.log
    echo "$actual_output" >> flow_results/test7_success.log
else
    echo "Test 7 failed"
    echo "Test 7 failed" >&2
    echo "Test: Maximum Input Test" > flow_results/test7_fail.log
    echo "Expected output:" >> flow_results/test7_fail.log
    echo "$expected_output" >> flow_results/test7_fail.log
    echo "Actual output:" >> flow_results/test7_fail.log
    echo "$actual_output" >> flow_results/test7_fail.log
    failed=1
fi
echo "Test 7 ended"

# Test 8: Binary Input Test
echo "Test 8 started"
actual_output=$(LD_PRELOAD=libtracer.so TRACE_OUTPUT=$PWD/flow_results/test8_trace.log ./vectors_t8 2>&1)
test8_status=$?
if [ $test8_status -eq 0 ] && [ "$actual_output" = "$expected_output" ]; then
    echo "Test 8 passed"
    echo "Test: Binary Input Test" > flow_results/test8_success.log
    echo "Actual output:" >> flow_results/test8_success.log
    echo "$actual_output" >> flow_results/test8_success.log
else
    echo "Test 8 failed"
    echo "Test 8 failed" >&2
    echo "Test: Binary Input Test" > flow_results/test8_fail.log
    echo "Expected output:" >> flow_results/test8_fail.log
    echo "$expected_output" >> flow_results/test8_fail.log
    echo "Actual output:" >> flow_results/test8_fail.log
    echo "$actual_output" >> flow_results/test8_fail.log
    failed=1
fi
echo "Test 8 ended"

# Test 9: Special Chars Test
echo "Test 9 started"
actual_output=$(LD_PRELOAD=libtracer.so TRACE_OUTPUT=$PWD/flow_results/test9_trace.log ./vectors_t9 2>&1)
test9_status=$?
if [ $test9_status -eq 0 ] && [ "$actual_output" = "$expected_output" ]; then
    echo "Test 9 passed"
    echo "Test: Special Chars Test" > flow_results/test9_success.log
    echo "Actual output:" >> flow_results/test9_success.log
    echo "$actual_output" >> flow_results/test9_success.log
else
    echo "Test 9 failed"
    echo "Test 9 failed" >&2
    echo "Test: Special Chars Test" > flow_results/test9_fail.log
    echo "Expected output:" >> flow_results/test9_fail.log
    echo "$expected_output" >> flow_results/test9_fail.log
    echo "Actual output:" >> flow_results/test9_fail.log
    echo "$actual_output" >> flow_results/test9_fail.log
    failed=1
fi
echo "Test 9 ended"

# Test 10: Numeric Key Test
echo "Test 10 started"
actual_output=$(LD_PRELOAD=libtracer.so TRACE_OUTPUT=$PWD/flow_results/test10_trace.log ./vectors_t10 2>&1)
test10_status=$?
if [ $test10_status -eq 0 ] && [ "$actual_output" = "$expected_output" ]; then
    echo "Test 10 passed"
    echo "Test: Numeric Key Test" > flow_results/test10_success.log
    echo "Actual output:" >> flow_results/test10_success.log
    echo "$actual_output" >> flow_results/test10_success.log
else
    echo "Test 10 failed"
    echo "Test 10 failed" >&2
    echo "Test: Numeric Key Test" > flow_results/test10_fail.log
    echo "Expected output:" >> flow_results/test10_fail.log
    echo "$expected_output" >> flow_results/test10_fail.log
    echo "Actual output:" >> flow_results/test10_fail.log
    echo "$actual_output" >> flow_results/test10_fail.log
    failed=1
fi
echo "Test 10 ended"

# Test 11: Boundary Test 1
echo "Test 11 started"
actual_output=$(LD_PRELOAD=libtracer.so TRACE_OUTPUT=$PWD/flow_results/test11_trace.log ./vectors_t11 2>&1)
test11_status=$?
if [ $test11_status -eq 0 ] && [ "$actual_output" = "$expected_output" ]; then
    echo "Test 11 passed"
    echo "Test: Boundary Test 1" > flow_results/test11_success.log
    echo "Actual output:" >> flow_results/test11_success.log
    echo "$actual_output" >> flow_results/test11_success.log
else
    echo "Test 11 failed"
    echo "Test 11 failed" >&2
    echo "Test: Boundary Test 1" > flow_results/test11_fail.log
    echo "Expected output:" >> flow_results/test11_fail.log
    echo "$expected_output" >> flow_results/test11_fail.log
    echo "Actual output:" >> flow_results/test11_fail.log
    echo "$actual_output" >> flow_results/test11_fail.log
    failed=1
fi
echo "Test 11 ended"

# Test 12: Boundary Test 2
echo "Test 12 started"
actual_output=$(LD_PRELOAD=libtracer.so TRACE_OUTPUT=$PWD/flow_results/test12_trace.log ./vectors_t12 2>&1)
test12_status=$?
if [ $test12_status -eq 0 ] && [ "$actual_output" = "$expected_output" ]; then
    echo "Test 12 passed"
    echo "Test: Boundary Test 2" > flow_results/test12_success.log
    echo "Actual output:" >> flow_results/test12_success.log
    echo "$actual_output" >> flow_results/test12_success.log
else
    echo "Test 12 failed"
    echo "Test 12 failed" >&2
    echo "Test: Boundary Test 2" > flow_results/test12_fail.log
    echo "Expected output:" >> flow_results/test12_fail.log
    echo "$expected_output" >> flow_results/test12_fail.log
    echo "Actual output:" >> flow_results/test12_fail.log
    echo "$actual_output" >> flow_results/test12_fail.log
    failed=1
fi
echo "Test 12 ended"

# Test 13: Mixed Input Test
echo "Test 13 started"
actual_output=$(LD_PRELOAD=libtracer.so TRACE_OUTPUT=$PWD/flow_results/test13_trace.log ./vectors_t13 2>&1)
test13_status=$?
if [ $test13_status -eq 0 ] && [ "$actual_output" = "$expected_output" ]; then
    echo "Test 13 passed"
    echo "Test: Mixed Input Test" > flow_results/test13_success.log
    echo "Actual output:" >> flow_results/test13_success.log
    echo "$actual_output" >> flow_results/test13_success.log
else
    echo "Test 13 failed"
    echo "Test 13 failed" >&2
    echo "Test: Mixed Input Test" > flow_results/test13_fail.log
    echo "Expected output:" >> flow_results/test13_fail.log
    echo "$expected_output" >> flow_results/test13_fail.log
    echo "Actual output:" >> flow_results/test13_fail.log
    echo "$actual_output" >> flow_results/test13_fail.log
    failed=1
fi
echo "Test 13 ended"

# Test 14: Sequential Input
echo "Test 14 started"
actual_output=$(LD_PRELOAD=libtracer.so TRACE_OUTPUT=$PWD/flow_results/test14_trace.log ./vectors_t14 2>&1)
test14_status=$?
if [ $test14_status -eq 0 ] && [ "$actual_output" = "$expected_output" ]; then
    echo "Test 14 passed"
    echo "Test: Sequential Input" > flow_results/test14_success.log
    echo "Actual output:" >> flow_results/test14_success.log
    echo "$actual_output" >> flow_results/test14_success.log
else
    echo "Test 14 failed"
    echo "Test 14 failed" >&2
    echo "Test: Sequential Input" > flow_results/test14_fail.log
    echo "Expected output:" >> flow_results/test14_fail.log
    echo "$expected_output" >> flow_results/test14_fail.log
    echo "Actual output:" >> flow_results/test14_fail.log
    echo "$actual_output" >> flow_results/test14_fail.log
    failed=1
fi
echo "Test 14 ended"

# Test 15: Unicode Key Test
echo "Test 15 started"
actual_output=$(LD_PRELOAD=libtracer.so TRACE_OUTPUT=$PWD/flow_results/test15_trace.log ./vectors_t15 2>&1)
test15_status=$?
if [ $test15_status -eq 0 ] && [ "$actual_output" = "$expected_output" ]; then
    echo "Test 15 passed"
    echo "Test: Unicode Key Test" > flow_results/test15_success.log
    echo "Actual output:" >> flow_results/test15_success.log
    echo "$actual_output" >> flow_results/test15_success.log
else
    echo "Test 15 failed"
    echo "Test 15 failed" >&2
    echo "Test: Unicode Key Test" > flow_results/test15_fail.log
    echo "Expected output:" >> flow_results/test15_fail.log
    echo "$expected_output" >> flow_results/test15_fail.log
    echo "Actual output:" >> flow_results/test15_fail.log
    echo "$actual_output" >> flow_results/test15_fail.log
    failed=1
fi
echo "Test 15 ended"

# Test 16: Long Input Test
echo "Test 16 started"
actual_output=$(LD_PRELOAD=libtracer.so TRACE_OUTPUT=$PWD/flow_results/test16_trace.log ./vectors_t16 2>&1)
test16_status=$?
if [ $test16_status -eq 0 ] && [ "$actual_output" = "$expected_output" ]; then
    echo "Test 16 passed"
    echo "Test: Long Input Test" > flow_results/test16_success.log
    echo "Actual output:" >> flow_results/test16_success.log
    echo "$actual_output" >> flow_results/test16_success.log
else
    echo "Test 16 failed"
    echo "Test 16 failed" >&2
    echo "Test: Long Input Test" > flow_results/test16_fail.log
    echo "Expected output:" >> flow_results/test16_fail.log
    echo "$expected_output" >> flow_results/test16_fail.log
    echo "Actual output:" >> flow_results/test16_fail.log
    echo "$actual_output" >> flow_results/test16_fail.log
    failed=1
fi
echo "Test 16 ended"

# Test 17: Short Input Test
echo "Test 17 started"
actual_output=$(LD_PRELOAD=libtracer.so TRACE_OUTPUT=$PWD/flow_results/test17_trace.log ./vectors_t17 2>&1)
test17_status=$?
if [ $test17_status -eq 0 ] && [ "$actual_output" = "$expected_output" ]; then
    echo "Test 17 passed"
    echo "Test: Short Input Test" > flow_results/test17_success.log
    echo "Actual output:" >> flow_results/test17_success.log
    echo "$actual_output" >> flow_results/test17_success.log
else
    echo "Test 17 failed"
    echo "Test 17 failed" >&2
    echo "Test: Short Input Test" > flow_results/test17_fail.log
    echo "Expected output:" >> flow_results/test17_fail.log
    echo "$expected_output" >> flow_results/test17_fail.log
    echo "Actual output:" >> flow_results/test17_fail.log
    echo "$actual_output" >> flow_results/test17_fail.log
    failed=1
fi
echo "Test 17 ended"

# Test 18: Random Input Test 1
echo "Test 18 started"
actual_output=$(LD_PRELOAD=libtracer.so TRACE_OUTPUT=$PWD/flow_results/test18_trace.log ./vectors_t18 2>&1)
test18_status=$?
if [ $test18_status -eq 0 ] && [ "$actual_output" = "$expected_output" ]; then
    echo "Test 18 passed"
    echo "Test: Random Input Test 1" > flow_results/test18_success.log
    echo "Actual output:" >> flow_results/test18_success.log
    echo "$actual_output" >> flow_results/test18_success.log
else
    echo "Test 18 failed"
    echo "Test 18 failed" >&2
    echo "Test: Random Input Test 1" > flow_results/test18_fail.log
    echo "Expected output:" >> flow_results/test18_fail.log
    echo "$expected_output" >> flow_results/test18_fail.log
    echo "Actual output:" >> flow_results/test18_fail.log
    echo "$actual_output" >> flow_results/test18_fail.log
    failed=1
fi
echo "Test 18 ended"

# Test 19: Random Input Test 2
echo "Test 19 started"
actual_output=$(LD_PRELOAD=libtracer.so TRACE_OUTPUT=$PWD/flow_results/test19_trace.log ./vectors_t19 2>&1)
test19_status=$?
if [ $test19_status -eq 0 ] && [ "$actual_output" = "$expected_output" ]; then
    echo "Test 19 passed"
    echo "Test: Random Input Test 2" > flow_results/test19_success.log
    echo "Actual output:" >> flow_results/test19_success.log
    echo "$actual_output" >> flow_results/test19_success.log
else
    echo "Test 19 failed"
    echo "Test 19 failed" >&2
    echo "Test: Random Input Test 2" > flow_results/test19_fail.log
    echo "Expected output:" >> flow_results/test19_fail.log
    echo "$expected_output" >> flow_results/test19_fail.log
    echo "Actual output:" >> flow_results/test19_fail.log
    echo "$actual_output" >> flow_results/test19_fail.log
    failed=1
fi
echo "Test 19 ended"

# Test 20: Edge Case Test
echo "Test 20 started"
actual_output=$(LD_PRELOAD=libtracer.so TRACE_OUTPUT=$PWD/flow_results/test20_trace.log ./vectors_t20 2>&1)
test20_status=$?
if [ $test20_status -eq 0 ] && [ "$actual_output" = "$expected_output" ]; then
    echo "Test 20 passed"
    echo "Test: Edge Case Test" > flow_results/test20_success.log
    echo "Actual output:" >> flow_results/test20_success.log
    echo "$actual_output" >> flow_results/test20_success.log
else
    echo "Test 20 failed"
    echo "Test 20 failed" >&2
    echo "Test: Edge Case Test" > flow_results/test20_fail.log
    echo "Expected output:" >> flow_results/test20_fail.log
    echo "$expected_output" >> flow_results/test20_fail.log
    echo "Actual output:" >> flow_results/test20_fail.log
    echo "$actual_output" >> flow_results/test20_fail.log
    failed=1
fi
echo "Test 20 ended"

exit $failed

