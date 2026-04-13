
#!/bin/bash

# Reformed test cases

failed=0

# Create results directory
rm -rf flow_results
mkdir -p flow_results

# Test case names corresponding to test numbers
test_cases=(
    "create_and_length"
    "create_with_specified_length"
    "string_concatenation"
    "sdscpy_against_longer_string"
    "sdscpy_against_shorter_string"
    "sdscatprintf_base_case"
    "sdscatprintf_with_null_char"
    "sdscatprintf_large_string"
    "sdscatfmt_base_case"
    "sdscatfmt_unsigned_numbers"
    "sdstrim_all_chars_match"
    "sdstrim_single_char_remains"
    "sdstrim_correctly_trims"
    "sdsrange_single_char"
    "sdsrange_from_start"
    "sdsrange_from_end"
    "sdsrange_empty_result"
    "sdsrange_beyond_length"
    "sdsrange_out_of_bounds"
    "sdscmp_greater"
    "sdscmp_equal"
    "sdscmp_less"
    "sdscatrepr"
    "sdsmakeroomfor"
)

# Test 1
echo "Test 1 started"
LD_PRELOAD=libtracer.so TRACE_OUTPUT=$PWD/flow_results/test1_trace.log ./sds-test_t1 "${test_cases[0]}" > flow_results/test1_output.log 2>&1
exit_code=$?
if [ $exit_code -eq 0 ]; then
    echo "Test 1 passed"
    cp flow_results/test1_output.log flow_results/test1_success.log
else
    echo "Test 1 failed"
    echo "Test 1 failed" >&2
    cp flow_results/test1_output.log flow_results/test1_fail.log
    failed=1
fi
echo "Test 1 ended"

# Test 2
echo "Test 2 started"
LD_PRELOAD=libtracer.so TRACE_OUTPUT=$PWD/flow_results/test2_trace.log ./sds-test_t2 "${test_cases[1]}" > flow_results/test2_output.log 2>&1
exit_code=$?
if [ $exit_code -eq 0 ]; then
    echo "Test 2 passed"
    cp flow_results/test2_output.log flow_results/test2_success.log
else
    echo "Test 2 failed"
    echo "Test 2 failed" >&2
    cp flow_results/test2_output.log flow_results/test2_fail.log
    failed=1
fi
echo "Test 2 ended"

# Test 3
echo "Test 3 started"
LD_PRELOAD=libtracer.so TRACE_OUTPUT=$PWD/flow_results/test3_trace.log ./sds-test_t3 "${test_cases[2]}" > flow_results/test3_output.log 2>&1
exit_code=$?
if [ $exit_code -eq 0 ]; then
    echo "Test 3 passed"
    cp flow_results/test3_output.log flow_results/test3_success.log
else
    echo "Test 3 failed"
    echo "Test 3 failed" >&2
    cp flow_results/test3_output.log flow_results/test3_fail.log
    failed=1
fi
echo "Test 3 ended"

# Test 4
echo "Test 4 started"
LD_PRELOAD=libtracer.so TRACE_OUTPUT=$PWD/flow_results/test4_trace.log ./sds-test_t4 "${test_cases[3]}" > flow_results/test4_output.log 2>&1
exit_code=$?
if [ $exit_code -eq 0 ]; then
    echo "Test 4 passed"
    cp flow_results/test4_output.log flow_results/test4_success.log
else
    echo "Test 4 failed"
    echo "Test 4 failed" >&2
    cp flow_results/test4_output.log flow_results/test4_fail.log
    failed=1
fi
echo "Test 4 ended"

# Test 5
echo "Test 5 started"
LD_PRELOAD=libtracer.so TRACE_OUTPUT=$PWD/flow_results/test5_trace.log ./sds-test_t5 "${test_cases[4]}" > flow_results/test5_output.log 2>&1
exit_code=$?
if [ $exit_code -eq 0 ]; then
    echo "Test 5 passed"
    cp flow_results/test5_output.log flow_results/test5_success.log
else
    echo "Test 5 failed"
    echo "Test 5 failed" >&2
    cp flow_results/test5_output.log flow_results/test5_fail.log
    failed=1
fi
echo "Test 5 ended"

# Test 6
echo "Test 6 started"
LD_PRELOAD=libtracer.so TRACE_OUTPUT=$PWD/flow_results/test6_trace.log ./sds-test_t6 "${test_cases[5]}" > flow_results/test6_output.log 2>&1
exit_code=$?
if [ $exit_code -eq 0 ]; then
    echo "Test 6 passed"
    cp flow_results/test6_output.log flow_results/test6_success.log
else
    echo "Test 6 failed"
    echo "Test 6 failed" >&2
    cp flow_results/test6_output.log flow_results/test6_fail.log
    failed=1
fi
echo "Test 6 ended"

# Test 7
echo "Test 7 started"
LD_PRELOAD=libtracer.so TRACE_OUTPUT=$PWD/flow_results/test7_trace.log ./sds-test_t7 "${test_cases[6]}" > flow_results/test7_output.log 2>&1
exit_code=$?
if [ $exit_code -eq 0 ]; then
    echo "Test 7 passed"
    cp flow_results/test7_output.log flow_results/test7_success.log
else
    echo "Test 7 failed"
    echo "Test 7 failed" >&2
    cp flow_results/test7_output.log flow_results/test7_fail.log
    failed=1
fi
echo "Test 7 ended"

# Test 8
echo "Test 8 started"
LD_PRELOAD=libtracer.so TRACE_OUTPUT=$PWD/flow_results/test8_trace.log ./sds-test_t8 "${test_cases[7]}" > flow_results/test8_output.log 2>&1
exit_code=$?
if [ $exit_code -eq 0 ]; then
    echo "Test 8 passed"
    cp flow_results/test8_output.log flow_results/test8_success.log
else
    echo "Test 8 failed"
    echo "Test 8 failed" >&2
    cp flow_results/test8_output.log flow_results/test8_fail.log
    failed=1
fi
echo "Test 8 ended"

# Test 9
echo "Test 9 started"
LD_PRELOAD=libtracer.so TRACE_OUTPUT=$PWD/flow_results/test9_trace.log ./sds-test_t9 "${test_cases[8]}" > flow_results/test9_output.log 2>&1
exit_code=$?
if [ $exit_code -eq 0 ]; then
    echo "Test 9 passed"
    cp flow_results/test9_output.log flow_results/test9_success.log
else
    echo "Test 9 failed"
    echo "Test 9 failed" >&2
    cp flow_results/test9_output.log flow_results/test9_fail.log
    failed=1
fi
echo "Test 9 ended"

# Test 10
echo "Test 10 started"
LD_PRELOAD=libtracer.so TRACE_OUTPUT=$PWD/flow_results/test10_trace.log ./sds-test_t10 "${test_cases[9]}" > flow_results/test10_output.log 2>&1
exit_code=$?
if [ $exit_code -eq 0 ]; then
    echo "Test 10 passed"
    cp flow_results/test10_output.log flow_results/test10_success.log
else
    echo "Test 10 failed"
    echo "Test 10 failed" >&2
    cp flow_results/test10_output.log flow_results/test10_fail.log
    failed=1
fi
echo "Test 10 ended"

# Test 11
echo "Test 11 started"
LD_PRELOAD=libtracer.so TRACE_OUTPUT=$PWD/flow_results/test11_trace.log ./sds-test_t11 "${test_cases[10]}" > flow_results/test11_output.log 2>&1
exit_code=$?
if [ $exit_code -eq 0 ]; then
    echo "Test 11 passed"
    cp flow_results/test11_output.log flow_results/test11_success.log
else
    echo "Test 11 failed"
    echo "Test 11 failed" >&2
    cp flow_results/test11_output.log flow_results/test11_fail.log
    failed=1
fi
echo "Test 11 ended"

# Test 12
echo "Test 12 started"
LD_PRELOAD=libtracer.so TRACE_OUTPUT=$PWD/flow_results/test12_trace.log ./sds-test_t12 "${test_cases[11]}" > flow_results/test12_output.log 2>&1
exit_code=$?
if [ $exit_code -eq 0 ]; then
    echo "Test 12 passed"
    cp flow_results/test12_output.log flow_results/test12_success.log
else
    echo "Test 12 failed"
    echo "Test 12 failed" >&2
    cp flow_results/test12_output.log flow_results/test12_fail.log
    failed=1
fi
echo "Test 12 ended"

# Test 13
echo "Test 13 started"
LD_PRELOAD=libtracer.so TRACE_OUTPUT=$PWD/flow_results/test13_trace.log ./sds-test_t13 "${test_cases[12]}" > flow_results/test13_output.log 2>&1
exit_code=$?
if [ $exit_code -eq 0 ]; then
    echo "Test 13 passed"
    cp flow_results/test13_output.log flow_results/test13_success.log
else
    echo "Test 13 failed"
    echo "Test 13 failed" >&2
    cp flow_results/test13_output.log flow_results/test13_fail.log
    failed=1
fi
echo "Test 13 ended"

# Test 14
echo "Test 14 started"
LD_PRELOAD=libtracer.so TRACE_OUTPUT=$PWD/flow_results/test14_trace.log ./sds-test_t14 "${test_cases[13]}" > flow_results/test14_output.log 2>&1
exit_code=$?
if [ $exit_code -eq 0 ]; then
    echo "Test 14 passed"
    cp flow_results/test14_output.log flow_results/test14_success.log
else
    echo "Test 14 failed"
    echo "Test 14 failed" >&2
    cp flow_results/test14_output.log flow_results/test14_fail.log
    failed=1
fi
echo "Test 14 ended"

# Test 15
echo "Test 15 started"
LD_PRELOAD=libtracer.so TRACE_OUTPUT=$PWD/flow_results/test15_trace.log ./sds-test_t15 "${test_cases[14]}" > flow_results/test15_output.log 2>&1
exit_code=$?
if [ $exit_code -eq 0 ]; then
    echo "Test 15 passed"
    cp flow_results/test15_output.log flow_results/test15_success.log
else
    echo "Test 15 failed"
    echo "Test 15 failed" >&2
    cp flow_results/test15_output.log flow_results/test15_fail.log
    failed=1
fi
echo "Test 15 ended"

# Test 16
echo "Test 16 started"
LD_PRELOAD=libtracer.so TRACE_OUTPUT=$PWD/flow_results/test16_trace.log ./sds-test_t16 "${test_cases[15]}" > flow_results/test16_output.log 2>&1
exit_code=$?
if [ $exit_code -eq 0 ]; then
    echo "Test 16 passed"
    cp flow_results/test16_output.log flow_results/test16_success.log
else
    echo "Test 16 failed"
    echo "Test 16 failed" >&2
    cp flow_results/test16_output.log flow_results/test16_fail.log
    failed=1
fi
echo "Test 16 ended"

# Test 17
echo "Test 17 started"
LD_PRELOAD=libtracer.so TRACE_OUTPUT=$PWD/flow_results/test17_trace.log ./sds-test_t17 "${test_cases[16]}" > flow_results/test17_output.log 2>&1
exit_code=$?
if [ $exit_code -eq 0 ]; then
    echo "Test 17 passed"
    cp flow_results/test17_output.log flow_results/test17_success.log
else
    echo "Test 17 failed"
    echo "Test 17 failed" >&2
    cp flow_results/test17_output.log flow_results/test17_fail.log
    failed=1
fi
echo "Test 17 ended"

# Test 18
echo "Test 18 started"
LD_PRELOAD=libtracer.so TRACE_OUTPUT=$PWD/flow_results/test18_trace.log ./sds-test_t18 "${test_cases[17]}" > flow_results/test18_output.log 2>&1
exit_code=$?
if [ $exit_code -eq 0 ]; then
    echo "Test 18 passed"
    cp flow_results/test18_output.log flow_results/test18_success.log
else
    echo "Test 18 failed"
    echo "Test 18 failed" >&2
    cp flow_results/test18_output.log flow_results/test18_fail.log
    failed=1
fi
echo "Test 18 ended"

# Test 19
echo "Test 19 started"
LD_PRELOAD=libtracer.so TRACE_OUTPUT=$PWD/flow_results/test19_trace.log ./sds-test_t19 "${test_cases[18]}" > flow_results/test19_output.log 2>&1
exit_code=$?
if [ $exit_code -eq 0 ]; then
    echo "Test 19 passed"
    cp flow_results/test19_output.log flow_results/test19_success.log
else
    echo "Test 19 failed"
    echo "Test 19 failed" >&2
    cp flow_results/test19_output.log flow_results/test19_fail.log
    failed=1
fi
echo "Test 19 ended"

# Test 20
echo "Test 20 started"
LD_PRELOAD=libtracer.so TRACE_OUTPUT=$PWD/flow_results/test20_trace.log ./sds-test_t20 "${test_cases[19]}" > flow_results/test20_output.log 2>&1
exit_code=$?
if [ $exit_code -eq 0 ]; then
    echo "Test 20 passed"
    cp flow_results/test20_output.log flow_results/test20_success.log
else
    echo "Test 20 failed"
    echo "Test 20 failed" >&2
    cp flow_results/test20_output.log flow_results/test20_fail.log
    failed=1
fi
echo "Test 20 ended"

# Test 21
echo "Test 21 started"
LD_PRELOAD=libtracer.so TRACE_OUTPUT=$PWD/flow_results/test21_trace.log ./sds-test_t21 "${test_cases[20]}" > flow_results/test21_output.log 2>&1
exit_code=$?
if [ $exit_code -eq 0 ]; then
    echo "Test 21 passed"
    cp flow_results/test21_output.log flow_results/test21_success.log
else
    echo "Test 21 failed"
    echo "Test 21 failed" >&2
    cp flow_results/test21_output.log flow_results/test21_fail.log
    failed=1
fi
echo "Test 21 ended"

# Test 22
echo "Test 22 started"
LD_PRELOAD=libtracer.so TRACE_OUTPUT=$PWD/flow_results/test22_trace.log ./sds-test_t22 "${test_cases[21]}" > flow_results/test22_output.log 2>&1
exit_code=$?
if [ $exit_code -eq 0 ]; then
    echo "Test 22 passed"
    cp flow_results/test22_output.log flow_results/test22_success.log
else
    echo "Test 22 failed"
    echo "Test 22 failed" >&2
    cp flow_results/test22_output.log flow_results/test22_fail.log
    failed=1
fi
echo "Test 22 ended"

# Test 23
echo "Test 23 started"
LD_PRELOAD=libtracer.so TRACE_OUTPUT=$PWD/flow_results/test23_trace.log ./sds-test_t23 "${test_cases[22]}" > flow_results/test23_output.log 2>&1
exit_code=$?
if [ $exit_code -eq 0 ]; then
    echo "Test 23 passed"
    cp flow_results/test23_output.log flow_results/test23_success.log
else
    echo "Test 23 failed"
    echo "Test 23 failed" >&2
    cp flow_results/test23_output.log flow_results/test23_fail.log
    failed=1
fi
echo "Test 23 ended"

# Test 24
echo "Test 24 started"
LD_PRELOAD=libtracer.so TRACE_OUTPUT=$PWD/flow_results/test24_trace.log ./sds-test_t24 "${test_cases[23]}" > flow_results/test24_output.log 2>&1
exit_code=$?
if [ $exit_code -eq 0 ]; then
    echo "Test 24 passed"
    cp flow_results/test24_output.log flow_results/test24_success.log
else
    echo "Test 24 failed"
    echo "Test 24 failed" >&2
    cp flow_results/test24_output.log flow_results/test24_fail.log
    failed=1
fi
echo "Test 24 ended"

exit $failed

