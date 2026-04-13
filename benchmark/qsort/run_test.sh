
#!/bin/bash

# Reformed test cases

failed=0

# Create results directory
rm -rf flow_results
mkdir -p flow_results

# --- Test 1: Sort an array in ascending order ---
echo "Test 1 started"
output=$(LD_PRELOAD=libtracer.so TRACE_OUTPUT=$PWD/flow_results/test1_trace.log ./bin/qsort_test_t1 1 5 2 8 1 3 2>&1)
exit_code=$?
sorted_line=$(echo "$output" | grep "After sorting:" | awk -F "After sorting: " '{print $2}')
expected="1 2 3 5 8 "
if [ "$sorted_line" = "$expected" ] && [ $exit_code -eq 0 ]; then
    echo "Test 1 passed"
    echo "Command: ./bin/qsort_test_t1 1 5 2 8 1 3" > flow_results/test1_success.log
    echo "Output: $output" >> flow_results/test1_success.log
    echo "Expected sorted array: $expected" >> flow_results/test1_success.log
    echo "Actual sorted array: $sorted_line" >> flow_results/test1_success.log
    echo "Exit code: $exit_code" >> flow_results/test1_success.log
else
    echo "Test 1 failed"
    echo "Test 1 failed" >&2
    failed=1
    echo "Command: ./bin/qsort_test_t1 1 5 2 8 1 3" > flow_results/test1_fail.log
    echo "Output: $output" >> flow_results/test1_fail.log
    echo "Expected sorted array: $expected" >> flow_results/test1_fail.log
    echo "Actual sorted array: $sorted_line" >> flow_results/test1_fail.log
    echo "Exit code: $exit_code" >> flow_results/test1_fail.log
fi
echo "Test 1 ended"

# --- Test 2: Sort an array with repeated values ---
echo "Test 2 started"
output=$(LD_PRELOAD=libtracer.so TRACE_OUTPUT=$PWD/flow_results/test2_trace.log ./bin/qsort_test_t2 2 4 1 3 4 2 2 2>&1)
exit_code=$?
sorted_line=$(echo "$output" | grep "After sorting:" | awk -F "After sorting: " '{print $2}')
expected="1 2 2 3 4 4 "
if [ "$sorted_line" = "$expected" ] && [ $exit_code -eq 0 ]; then
    echo "Test 2 passed"
    echo "Command: ./bin/qsort_test_t2 2 4 1 3 4 2 2" > flow_results/test2_success.log
    echo "Output: $output" >> flow_results/test2_success.log
    echo "Expected sorted array: $expected" >> flow_results/test2_success.log
    echo "Actual sorted array: $sorted_line" >> flow_results/test2_success.log
    echo "Exit code: $exit_code" >> flow_results/test2_success.log
else
    echo "Test 2 failed"
    echo "Test 2 failed" >&2
    failed=1
    echo "Command: ./bin/qsort_test_t2 2 4 1 3 4 2 2" > flow_results/test2_fail.log
    echo "Output: $output" >> flow_results/test2_fail.log
    echo "Expected sorted array: $expected" >> flow_results/test2_fail.log
    echo "Actual sorted array: $sorted_line" >> flow_results/test2_fail.log
    echo "Exit code: $exit_code" >> flow_results/test2_fail.log
fi
echo "Test 2 ended"

# --- Test 3: Sort an already sorted array ---
echo "Test 3 started"
output=$(LD_PRELOAD=libtracer.so TRACE_OUTPUT=$PWD/flow_results/test3_trace.log ./bin/qsort_test_t3 3 1 2 3 4 5 2>&1)
exit_code=$?
sorted_line=$(echo "$output" | grep "After sorting:" | awk -F "After sorting: " '{print $2}')
expected="1 2 3 4 5 "
if [ "$sorted_line" = "$expected" ] && [ $exit_code -eq 0 ]; then
    echo "Test 3 passed"
    echo "Command: ./bin/qsort_test_t3 3 1 2 3 4 5" > flow_results/test3_success.log
    echo "Output: $output" >> flow_results/test3_success.log
    echo "Expected sorted array: $expected" >> flow_results/test3_success.log
    echo "Actual sorted array: $sorted_line" >> flow_results/test3_success.log
    echo "Exit code: $exit_code" >> flow_results/test3_success.log
else
    echo "Test 3 failed"
    echo "Test 3 failed" >&2
    failed=1
    echo "Command: ./bin/qsort_test_t3 3 1 2 3 4 5" > flow_results/test3_fail.log
    echo "Output: $output" >> flow_results/test3_fail.log
    echo "Expected sorted array: $expected" >> flow_results/test3_fail.log
    echo "Actual sorted array: $sorted_line" >> flow_results/test3_fail.log
    echo "Exit code: $exit_code" >> flow_results/test3_fail.log
fi
echo "Test 3 ended"

# --- Test 4: Sort an array with one element ---
echo "Test 4 started"
output=$(LD_PRELOAD=libtracer.so TRACE_OUTPUT=$PWD/flow_results/test4_trace.log ./bin/qsort_test_t4 4 42 2>&1)
exit_code=$?
sorted_line=$(echo "$output" | grep "After sorting:" | awk -F "After sorting: " '{print $2}')
expected="42 "
if [ "$sorted_line" = "$expected" ] && [ $exit_code -eq 0 ]; then
    echo "Test 4 passed"
    echo "Command: ./bin/qsort_test_t4 4 42" > flow_results/test4_success.log
    echo "Output: $output" >> flow_results/test4_success.log
    echo "Expected sorted array: $expected" >> flow_results/test4_success.log
    echo "Actual sorted array: $sorted_line" >> flow_results/test4_success.log
    echo "Exit code: $exit_code" >> flow_results/test4_success.log
else
    echo "Test 4 failed"
    echo "Test 4 failed" >&2
    failed=1
    echo "Command: ./bin/qsort_test_t4 4 42" > flow_results/test4_fail.log
    echo "Output: $output" >> flow_results/test4_fail.log
    echo "Expected sorted array: $expected" >> flow_results/test4_fail.log
    echo "Actual sorted array: $sorted_line" >> flow_results/test4_fail.log
    echo "Exit code: $exit_code" >> flow_results/test4_fail.log
fi
echo "Test 4 ended"

# --- Test 5: Sort an empty array ---
echo "Test 5 started"
output=$(LD_PRELOAD=libtracer.so TRACE_OUTPUT=$PWD/flow_results/test5_trace.log ./bin/qsort_test_t5 5 2>&1)
exit_code=$?
sorted_line=$(echo "$output" | grep "After sorting:" | awk -F "After sorting: " '{print $2}')
expected=""
if [ "$sorted_line" = "$expected" ] && [ $exit_code -eq 0 ]; then
    echo "Test 5 passed"
    echo "Command: ./bin/qsort_test_t5 5" > flow_results/test5_success.log
    echo "Output: $output" >> flow_results/test5_success.log
    echo "Expected sorted array: $expected" >> flow_results/test5_success.log
    echo "Actual sorted array: $sorted_line" >> flow_results/test5_success.log
    echo "Exit code: $exit_code" >> flow_results/test5_success.log
else
    echo "Test 5 failed"
    echo "Test 5 failed" >&2
    failed=1
    echo "Command: ./bin/qsort_test_t5 5" > flow_results/test5_fail.log
    echo "Output: $output" >> flow_results/test5_fail.log
    echo "Expected sorted array: $expected" >> flow_results/test5_fail.log
    echo "Actual sorted array: $sorted_line" >> flow_results/test5_fail.log
    echo "Exit code: $exit_code" >> flow_results/test5_fail.log
fi
echo "Test 5 ended"

# --- Test 6: Array with negative numbers ---
echo "Test 6 started"
output=$(LD_PRELOAD=libtracer.so TRACE_OUTPUT=$PWD/flow_results/test6_trace.log ./bin/qsort_test_t6 6 -5 12 -10 0 8 -15 3 2>&1)
exit_code=$?
sorted_line=$(echo "$output" | grep "After sorting:" | awk -F "After sorting: " '{print $2}')
expected="-15 -10 -5 0 3 8 12 "
if [ "$sorted_line" = "$expected" ] && [ $exit_code -eq 0 ]; then
    echo "Test 6 passed"
    echo "Command: ./bin/qsort_test_t6 6 -5 12 -10 0 8 -15 3" > flow_results/test6_success.log
    echo "Output: $output" >> flow_results/test6_success.log
    echo "Expected sorted array: $expected" >> flow_results/test6_success.log
    echo "Actual sorted array: $sorted_line" >> flow_results/test6_success.log
    echo "Exit code: $exit_code" >> flow_results/test6_success.log
else
    echo "Test 6 failed"
    echo "Test 6 failed" >&2
    failed=1
    echo "Command: ./bin/qsort_test_t6 6 -5 12 -10 0 8 -15 3" > flow_results/test6_fail.log
    echo "Output: $output" >> flow_results/test6_fail.log
    echo "Expected sorted array: $expected" >> flow_results/test6_fail.log
    echo "Actual sorted array: $sorted_line" >> flow_results/test6_fail.log
    echo "Exit code: $exit_code" >> flow_results/test6_fail.log
fi
echo "Test 6 ended"

# --- Test 7: Empty array ---
echo "Test 7 started"
output=$(LD_PRELOAD=libtracer.so TRACE_OUTPUT=$PWD/flow_results/test7_trace.log ./bin/qsort_test_t7 7 2>&1)
exit_code=$?
sorted_line=$(echo "$output" | grep "After sorting:" | awk -F "After sorting: " '{print $2}')
expected=""
if [ "$sorted_line" = "$expected" ] && [ $exit_code -eq 0 ]; then
    echo "Test 7 passed"
    echo "Command: ./bin/qsort_test_t7 7" > flow_results/test7_success.log
    echo "Output: $output" >> flow_results/test7_success.log
    echo "Expected sorted array: $expected" >> flow_results/test7_success.log
    echo "Actual sorted array: $sorted_line" >> flow_results/test7_success.log
    echo "Exit code: $exit_code" >> flow_results/test7_success.log
else
    echo "Test 7 failed"
    echo "Test 7 failed" >&2
    failed=1
    echo "Command: ./bin/qsort_test_t7 7" > flow_results/test7_fail.log
    echo "Output: $output" >> flow_results/test7_fail.log
    echo "Expected sorted array: $expected" >> flow_results/test7_fail.log
    echo "Actual sorted array: $sorted_line" >> flow_results/test7_fail.log
    echo "Exit code: $exit_code" >> flow_results/test7_fail.log
fi
echo "Test 7 ended"

# --- Test 8: Large array (20 elements) ---
echo "Test 8 started"
output=$(LD_PRELOAD=libtracer.so TRACE_OUTPUT=$PWD/flow_results/test8_trace.log ./bin/qsort_test_t8 8 45 23 68 12 90 34 56 78 11 99 22 33 44 55 66 77 88 98 21 32 2>&1)
exit_code=$?
sorted_line=$(echo "$output" | grep "After sorting:" | awk -F "After sorting: " '{print $2}')
expected="11 12 21 22 23 32 33 34 44 45 55 56 66 68 77 78 88 90 98 99 "
if [ "$sorted_line" = "$expected" ] && [ $exit_code -eq 0 ]; then
    echo "Test 8 passed"
    echo "Command: ./bin/qsort_test_t8 8 45 23 68 12 90 34 56 78 11 99 22 33 44 55 66 77 88 98 21 32" > flow_results/test8_success.log
    echo "Output: $output" >> flow_results/test8_success.log
    echo "Expected sorted array: $expected" >> flow_results/test8_success.log
    echo "Actual sorted array: $sorted_line" >> flow_results/test8_success.log
    echo "Exit code: $exit_code" >> flow_results/test8_success.log
else
    echo "Test 8 failed"
    echo "Test 8 failed" >&2
    failed=1
    echo "Command: ./bin/qsort_test_t8 8 45 23 68 12 90 34 56 78 11 99 22 33 44 55 66 77 88 98 21 32" > flow_results/test8_fail.log
    echo "Output: $output" >> flow_results/test8_fail.log
    echo "Expected sorted array: $expected" >> flow_results/test8_fail.log
    echo "Actual sorted array: $sorted_line" >> flow_results/test8_fail.log
    echo "Exit code: $exit_code" >> flow_results/test8_fail.log
fi
echo "Test 8 ended"

# --- Test 9: Array with all zeros ---
echo "Test 9 started"
output=$(LD_PRELOAD=libtracer.so TRACE_OUTPUT=$PWD/flow_results/test9_trace.log ./bin/qsort_test_t9 9 0 0 0 0 0 2>&1)
exit_code=$?
sorted_line=$(echo "$output" | grep "After sorting:" | awk -F "After sorting: " '{print $2}')
expected="0 0 0 0 0 "
if [ "$sorted_line" = "$expected" ] && [ $exit_code -eq 0 ]; then
    echo "Test 9 passed"
    echo "Command: ./bin/qsort_test_t9 9 0 0 0 0 0" > flow_results/test9_success.log
    echo "Output: $output" >> flow_results/test9_success.log
    echo "Expected sorted array: $expected" >> flow_results/test9_success.log
    echo "Actual sorted array: $sorted_line" >> flow_results/test9_success.log
    echo "Exit code: $exit_code" >> flow_results/test9_success.log
else
    echo "Test 9 failed"
    echo "Test 9 failed" >&2
    failed=1
    echo "Command: ./bin/qsort_test_t9 9 0 0 0 0 0" > flow_results/test9_fail.log
    echo "Output: $output" >> flow_results/test9_fail.log
    echo "Expected sorted array: $expected" >> flow_results/test9_fail.log
    echo "Actual sorted array: $sorted_line" >> flow_results/test9_fail.log
    echo "Exit code: $exit_code" >> flow_results/test9_fail.log
fi
echo "Test 9 ended"

# --- Test 10: Array with all the same value ---
echo "Test 10 started"
output=$(LD_PRELOAD=libtracer.so TRACE_OUTPUT=$PWD/flow_results/test10_trace.log ./bin/qsort_test_t10 10 42 42 42 42 42 2>&1)
exit_code=$?
sorted_line=$(echo "$output" | grep "After sorting:" | awk -F "After sorting: " '{print $2}')
expected="42 42 42 42 42 "
if [ "$sorted_line" = "$expected" ] && [ $exit_code -eq 0 ]; then
    echo "Test 10 passed"
    echo "Command: ./bin/qsort_test_t10 10 42 42 42 42 42" > flow_results/test10_success.log
    echo "Output: $output" >> flow_results/test10_success.log
    echo "Expected sorted array: $expected" >> flow_results/test10_success.log
    echo "Actual sorted array: $sorted_line" >> flow_results/test10_success.log
    echo "Exit code: $exit_code" >> flow_results/test10_success.log
else
    echo "Test 10 failed"
    echo "Test 10 failed" >&2
    failed=1
    echo "Command: ./bin/qsort_test_t10 10 42 42 42 42 42" > flow_results/test10_fail.log
    echo "Output: $output" >> flow_results/test10_fail.log
    echo "Expected sorted array: $expected" >> flow_results/test10_fail.log
    echo "Actual sorted array: $sorted_line" >> flow_results/test10_fail.log
    echo "Exit code: $exit_code" >> flow_results/test10_fail.log
fi
echo "Test 10 ended"

# --- Test 11: Array with alternating values ---
echo "Test 11 started"
output=$(LD_PRELOAD=libtracer.so TRACE_OUTPUT=$PWD/flow_results/test11_trace.log ./bin/qsort_test_t11 11 1 2 1 2 1 2 2>&1)
exit_code=$?
sorted_line=$(echo "$output" | grep "After sorting:" | awk -F "After sorting: " '{print $2}')
expected="1 1 1 2 2 2 "
if [ "$sorted_line" = "$expected" ] && [ $exit_code -eq 0 ]; then
    echo "Test 11 passed"
    echo "Command: ./bin/qsort_test_t11 11 1 2 1 2 1 2" > flow_results/test11_success.log
    echo "Output: $output" >> flow_results/test11_success.log
    echo "Expected sorted array: $expected" >> flow_results/test11_success.log
    echo "Actual sorted array: $sorted_line" >> flow_results/test11_success.log
    echo "Exit code: $exit_code" >> flow_results/test11_success.log
else
    echo "Test 11 failed"
    echo "Test 11 failed" >&2
    failed=1
    echo "Command: ./bin/qsort_test_t11 11 1 2 1 2 1 2" > flow_results/test11_fail.log
    echo "Output: $output" >> flow_results/test11_fail.log
    echo "Expected sorted array: $expected" >> flow_results/test11_fail.log
    echo "Actual sorted array: $sorted_line" >> flow_results/test11_fail.log
    echo "Exit code: $exit_code" >> flow_results/test11_fail.log
fi
echo "Test 11 ended"

# --- Test 12: Array with extreme values ---
echo "Test 12 started"
output=$(LD_PRELOAD=libtracer.so TRACE_OUTPUT=$PWD/flow_results/test12_trace.log ./bin/qsort_test_t12 12 2147483647 -2147483648 0 2>&1)
exit_code=$?
sorted_line=$(echo "$output" | grep "After sorting:" | awk -F "After sorting: " '{print $2}')
expected="-2147483648 0 2147483647 "
if [ "$sorted_line" = "$expected" ] && [ $exit_code -eq 0 ]; then
    echo "Test 12 passed"
    echo "Command: ./bin/qsort_test_t12 12 2147483647 -2147483648 0" > flow_results/test12_success.log
    echo "Output: $output" >> flow_results/test12_success.log
    echo "Expected sorted array: $expected" >> flow_results/test12_success.log
    echo "Actual sorted array: $sorted_line" >> flow_results/test12_success.log
    echo "Exit code: $exit_code" >> flow_results/test12_success.log
else
    echo "Test 12 failed"
    echo "Test 12 failed" >&2
    failed=1
    echo "Command: ./bin/qsort_test_t12 12 2147483647 -2147483648 0" > flow_results/test12_fail.log
    echo "Output: $output" >> flow_results/test12_fail.log
    echo "Expected sorted array: $expected" >> flow_results/test12_fail.log
    echo "Actual sorted array: $sorted_line" >> flow_results/test12_fail.log
    echo "Exit code: $exit_code" >> flow_results/test12_fail.log
fi
echo "Test 12 ended"

# --- Test 13: Near-sorted array ---
echo "Test 13 started"
output=$(LD_PRELOAD=libtracer.so TRACE_OUTPUT=$PWD/flow_results/test13_trace.log ./bin/qsort_test_t13 13 1 2 3 5 4 6 2>&1)
exit_code=$?
sorted_line=$(echo "$output" | grep "After sorting:" | awk -F "After sorting: " '{print $2}')
expected="1 2 3 4 5 6 "
if [ "$sorted_line" = "$expected" ] && [ $exit_code -eq 0 ]; then
    echo "Test 13 passed"
    echo "Command: ./bin/qsort_test_t13 13 1 2 3 5 4 6" > flow_results/test13_success.log
    echo "Output: $output" >> flow_results/test13_success.log
    echo "Expected sorted array: $expected" >> flow_results/test13_success.log
    echo "Actual sorted array: $sorted_line" >> flow_results/test13_success.log
    echo "Exit code: $exit_code" >> flow_results/test13_success.log
else
    echo "Test 13 failed"
    echo "Test 13 failed" >&2
    failed=1
    echo "Command: ./bin/qsort_test_t13 13 1 2 3 5 4 6" > flow_results/test13_fail.log
    echo "Output: $output" >> flow_results/test13_fail.log
    echo "Expected sorted array: $expected" >> flow_results/test13_fail.log
    echo "Actual sorted array: $sorted_line" >> flow_results/test13_fail.log
    echo "Exit code: $exit_code" >> flow_results/test13_fail.log
fi
echo "Test 13 ended"

# --- Test 14: Array with repeated min/max values ---
echo "Test 14 started"
output=$(LD_PRELOAD=libtracer.so TRACE_OUTPUT=$PWD/flow_results/test14_trace.log ./bin/qsort_test_t14 14 100 1 100 1 100 1 2>&1)
exit_code=$?
sorted_line=$(echo "$output" | grep "After sorting:" | awk -F "After sorting: " '{print $2}')
expected="1 1 1 100 100 100 "
if [ "$sorted_line" = "$expected" ] && [ $exit_code -eq 0 ]; then
    echo "Test 14 passed"
    echo "Command: ./bin/qsort_test_t14 14 100 1 100 1 100 1" > flow_results/test14_success.log
    echo "Output: $output" >> flow_results/test14_success.log
    echo "Expected sorted array: $expected" >> flow_results/test14_success.log
    echo "Actual sorted array: $sorted_line" >> flow_results/test14_success.log
    echo "Exit code: $exit_code" >> flow_results/test14_success.log
else
    echo "Test 14 failed"
    echo "Test 14 failed" >&2
    failed=1
    echo "Command: ./bin/qsort_test_t14 14 100 1 100 1 100 1" > flow_results/test14_fail.log
    echo "Output: $output" >> flow_results/test14_fail.log
    echo "Expected sorted array: $expected" >> flow_results/test14_fail.log
    echo "Actual sorted array: $sorted_line" >> flow_results/test14_fail.log
    echo "Exit code: $exit_code" >> flow_results/test14_fail.log
fi
echo "Test 14 ended"

# --- Test 15: Small random array ---
echo "Test 15 started"
output=$(LD_PRELOAD=libtracer.so TRACE_OUTPUT=$PWD/flow_results/test15_trace.log ./bin/qsort_test_t15 15 42 17 35 9 24 2>&1)
exit_code=$?
sorted_line=$(echo "$output" | grep "After sorting:" | awk -F "After sorting: " '{print $2}')
expected="9 17 24 35 42 "
if [ "$sorted_line" = "$expected" ] && [ $exit_code -eq 0 ]; then
    echo "Test 15 passed"
    echo "Command: ./bin/qsort_test_t15 15 42 17 35 9 24" > flow_results/test15_success.log
    echo "Output: $output" >> flow_results/test15_success.log
    echo "Expected sorted array: $expected" >> flow_results/test15_success.log
    echo "Actual sorted array: $sorted_line" >> flow_results/test15_success.log
    echo "Exit code: $exit_code" >> flow_results/test15_success.log
else
    echo "Test 15 failed"
    echo "Test 15 failed" >&2
    failed=1
    echo "Command: ./bin/qsort_test_t15 15 42 17 35 9 24" > flow_results/test15_fail.log
    echo "Output: $output" >> flow_results/test15_fail.log
    echo "Expected sorted array: $expected" >> flow_results/test15_fail.log
    echo "Actual sorted array: $sorted_line" >> flow_results/test15_fail.log
    echo "Exit code: $exit_code" >> flow_results/test15_fail.log
fi
echo "Test 15 ended"

# --- Test 16: Mix of positive and negative even numbers ---
echo "Test 16 started"
output=$(LD_PRELOAD=libtracer.so TRACE_OUTPUT=$PWD/flow_results/test16_trace.log ./bin/qsort_test_t16 16 2 -4 6 -8 10 -12 2>&1)
exit_code=$?
sorted_line=$(echo "$output" | grep "After sorting:" | awk -F "After sorting: " '{print $2}')
expected="-12 -8 -4 2 6 10 "
if [ "$sorted_line" = "$expected" ] && [ $exit_code -eq 0 ]; then
    echo "Test 16 passed"
    echo "Command: ./bin/qsort_test_t16 16 2 -4 6 -8 10 -12" > flow_results/test16_success.log
    echo "Output: $output" >> flow_results/test16_success.log
    echo "Expected sorted array: $expected" >> flow_results/test16_success.log
    echo "Actual sorted array: $sorted_line" >> flow_results/test16_success.log
    echo "Exit code: $exit_code" >> flow_results/test16_success.log
else
    echo "Test 16 failed"
    echo "Test 16 failed" >&2
    failed=1
    echo "Command: ./bin/qsort_test_t16 16 2 -4 6 -8 10 -12" > flow_results/test16_fail.log
    echo "Output: $output" >> flow_results/test16_fail.log
    echo "Expected sorted array: $expected" >> flow_results/test16_fail.log
    echo "Actual sorted array: $sorted_line" >> flow_results/test16_fail.log
    echo "Exit code: $exit_code" >> flow_results/test16_fail.log
fi
echo "Test 16 ended"

# --- Test 17: Mix of positive and negative odd numbers ---
echo "Test 17 started"
output=$(LD_PRELOAD=libtracer.so TRACE_OUTPUT=$PWD/flow_results/test17_trace.log ./bin/qsort_test_t17 17 1 -3 5 -7 9 -11 2>&1)
exit_code=$?
sorted_line=$(echo "$output" | grep "After sorting:" | awk -F "After sorting: " '{print $2}')
expected="-11 -7 -3 1 5 9 "
if [ "$sorted_line" = "$expected" ] && [ $exit_code -eq 0 ]; then
    echo "Test 17 passed"
    echo "Command: ./bin/qsort_test_t17 17 1 -3 5 -7 9 -11" > flow_results/test17_success.log
    echo "Output: $output" >> flow_results/test17_success.log
    echo "Expected sorted array: $expected" >> flow_results/test17_success.log
    echo "Actual sorted array: $sorted_line" >> flow_results/test17_success.log
    echo "Exit code: $exit_code" >> flow_results/test17_success.log
else
    echo "Test 17 failed"
    echo "Test 17 failed" >&2
    failed=1
    echo "Command: ./bin/qsort_test_t17 17 1 -3 5 -7 9 -11" > flow_results/test17_fail.log
    echo "Output: $output" >> flow_results/test17_fail.log
    echo "Expected sorted array: $expected" >> flow_results/test17_fail.log
    echo "Actual sorted array: $sorted_line" >> flow_results/test17_fail.log
    echo "Exit code: $exit_code" >> flow_results/test17_fail.log
fi
echo "Test 17 ended"

# --- Test 18: Array with sequential pairs ---
echo "Test 18 started"
output=$(LD_PRELOAD=libtracer.so TRACE_OUTPUT=$PWD/flow_results/test18_trace.log ./bin/qsort_test_t18 18 5 6 3 4 1 2 7 8 2>&1)
exit_code=$?
sorted_line=$(echo "$output" | grep "After sorting:" | awk -F "After sorting: " '{print $2}')
expected="1 2 3 4 5 6 7 8 "
if [ "$sorted_line" = "$expected" ] && [ $exit_code -eq 0 ]; then
    echo "Test 18 passed"
    echo "Command: ./bin/qsort_test_t18 18 5 6 3 4 1 2 7 8" > flow_results/test18_success.log
    echo "Output: $output" >> flow_results/test18_success.log
    echo "Expected sorted array: $expected" >> flow_results/test18_success.log
    echo "Actual sorted array: $sorted_line" >> flow_results/test18_success.log
    echo "Exit code: $exit_code" >> flow_results/test18_success.log
else
    echo "Test 18 failed"
    echo "Test 18 failed" >&2
    failed=1
    echo "Command: ./bin/qsort_test_t18 18 5 6 3 4 1 2 7 8" > flow_results/test18_fail.log
    echo "Output: $output" >> flow_results/test18_fail.log
    echo "Expected sorted array: $expected" >> flow_results/test18_fail.log
    echo "Actual sorted array: $sorted_line" >> flow_results/test18_fail.log
    echo "Exit code: $exit_code" >> flow_results/test18_fail.log
fi
echo "Test 18 ended"

# --- Test 19: Palindromic array ---
echo "Test 19 started"
output=$(LD_PRELOAD=libtracer.so TRACE_OUTPUT=$PWD/flow_results/test19_trace.log ./bin/qsort_test_t19 19 1 3 5 7 5 3 1 2>&1)
exit_code=$?
sorted_line=$(echo "$output" | grep "After sorting:" | awk -F "After sorting: " '{print $2}')
expected="1 1 3 3 5 5 7 "
if [ "$sorted_line" = "$expected" ] && [ $exit_code -eq 0 ]; then
    echo "Test 19 passed"
    echo "Command: ./bin/qsort_test_t19 19 1 3 5 7 5 3 1" > flow_results/test19_success.log
    echo "Output: $output" >> flow_results/test19_success.log
    echo "Expected sorted array: $expected" >> flow_results/test19_success.log
    echo "Actual sorted array: $sorted_line" >> flow_results/test19_success.log
    echo "Exit code: $exit_code" >> flow_results/test19_success.log
else
    echo "Test 19 failed"
    echo "Test 19 failed" >&2
    failed=1
    echo "Command: ./bin/qsort_test_t19 19 1 3 5 7 5 3 1" > flow_results/test19_fail.log
    echo "Output: $output" >> flow_results/test19_fail.log
    echo "Expected sorted array: $expected" >> flow_results/test19_fail.log
    echo "Actual sorted array: $sorted_line" >> flow_results/test19_fail.log
    echo "Exit code: $exit_code" >> flow_results/test19_fail.log
fi
echo "Test 19 ended"

# --- Test 20: Two-element array ---
echo "Test 20 started"
output=$(LD_PRELOAD=libtracer.so TRACE_OUTPUT=$PWD/flow_results/test20_trace.log ./bin/qsort_test_t20 20 5 1 2>&1)
exit_code=$?
sorted_line=$(echo "$output" | grep "After sorting:" | awk -F "After sorting: " '{print $2}')
expected="1 5 "
if [ "$sorted_line" = "$expected" ] && [ $exit_code -eq 0 ]; then
    echo "Test 20 passed"
    echo "Command: ./bin/qsort_test_t20 20 5 1" > flow_results/test20_success.log
    echo "Output: $output" >> flow_results/test20_success.log
    echo "Expected sorted array: $expected" >> flow_results/test20_success.log
    echo "Actual sorted array: $sorted_line" >> flow_results/test20_success.log
    echo "Exit code: $exit_code" >> flow_results/test20_success.log
else
    echo "Test 20 failed"
    echo "Test 20 failed" >&2
    failed=1
    echo "Command: ./bin/qsort_test_t20 20 5 1" > flow_results/test20_fail.log
    echo "Output: $output" >> flow_results/test20_fail.log
    echo "Expected sorted array: $expected" >> flow_results/test20_fail.log
    echo "Actual sorted array: $sorted_line" >> flow_results/test20_fail.log
    echo "Exit code: $exit_code" >> flow_results/test20_fail.log
fi
echo "Test 20 ended"

exit $failed

