
#!/bin/bash

# Reformed test cases
failed=0

mkdir -p genifai_results

# Test 1: Original test.c - full assertion-based test
echo "Test 1 started"
LD_PRELOAD=libtracer.so TRACE_OUTPUT=$PWD/genifai_results/test1_trace.log ./url-test_t1 > genifai_results/test1_output.log 2>&1
ret=$?
if [ $ret -eq 0 ]; then
    echo "Test 1 passed"
    cp genifai_results/test1_output.log genifai_results/test1_success.log
else
    echo "Test 1 failed"
    echo "Test 1 failed" >&2
    cp genifai_results/test1_output.log genifai_results/test1_fail.log
    failed=1
fi
echo "Test 1 ended"

# Test 2: HTTP URL protocol value
echo "Test 2 started"
LD_PRELOAD=libtracer.so TRACE_OUTPUT=$PWD/genifai_results/test2_trace.log ./url-test_t2 > genifai_results/test2_output.log 2>&1
ret=$?
if [ $ret -eq 0 ]; then
    echo "Test 2 passed"
    cp genifai_results/test2_output.log genifai_results/test2_success.log
else
    echo "Test 2 failed"
    echo "Test 2 failed" >&2
    cp genifai_results/test2_output.log genifai_results/test2_fail.log
    failed=1
fi
echo "Test 2 ended"

# Test 3: HTTP URL auth value
echo "Test 3 started"
LD_PRELOAD=libtracer.so TRACE_OUTPUT=$PWD/genifai_results/test3_trace.log ./url-test_t3 > genifai_results/test3_output.log 2>&1
ret=$?
if [ $ret -eq 0 ]; then
    echo "Test 3 passed"
    cp genifai_results/test3_output.log genifai_results/test3_success.log
else
    echo "Test 3 failed"
    echo "Test 3 failed" >&2
    cp genifai_results/test3_output.log genifai_results/test3_fail.log
    failed=1
fi
echo "Test 3 ended"

# Test 4: HTTP URL hostname value
echo "Test 4 started"
LD_PRELOAD=libtracer.so TRACE_OUTPUT=$PWD/genifai_results/test4_trace.log ./url-test_t4 > genifai_results/test4_output.log 2>&1
ret=$?
if [ $ret -eq 0 ]; then
    echo "Test 4 passed"
    cp genifai_results/test4_output.log genifai_results/test4_success.log
else
    echo "Test 4 failed"
    echo "Test 4 failed" >&2
    cp genifai_results/test4_output.log genifai_results/test4_fail.log
    failed=1
fi
echo "Test 4 ended"

# Test 5: HTTP URL host value
echo "Test 5 started"
LD_PRELOAD=libtracer.so TRACE_OUTPUT=$PWD/genifai_results/test5_trace.log ./url-test_t5 > genifai_results/test5_output.log 2>&1
ret=$?
if [ $ret -eq 0 ]; then
    echo "Test 5 passed"
    cp genifai_results/test5_output.log genifai_results/test5_success.log
else
    echo "Test 5 failed"
    echo "Test 5 failed" >&2
    cp genifai_results/test5_output.log genifai_results/test5_fail.log
    failed=1
fi
echo "Test 5 ended"

# Test 6: HTTP URL pathname value
echo "Test 6 started"
LD_PRELOAD=libtracer.so TRACE_OUTPUT=$PWD/genifai_results/test6_trace.log ./url-test_t6 > genifai_results/test6_output.log 2>&1
ret=$?
if [ $ret -eq 0 ]; then
    echo "Test 6 passed"
    cp genifai_results/test6_output.log genifai_results/test6_success.log
else
    echo "Test 6 failed"
    echo "Test 6 failed" >&2
    cp genifai_results/test6_output.log genifai_results/test6_fail.log
    failed=1
fi
echo "Test 6 ended"

# Test 7: HTTP URL path value
echo "Test 7 started"
LD_PRELOAD=libtracer.so TRACE_OUTPUT=$PWD/genifai_results/test7_trace.log ./url-test_t7 > genifai_results/test7_output.log 2>&1
ret=$?
if [ $ret -eq 0 ]; then
    echo "Test 7 passed"
    cp genifai_results/test7_output.log genifai_results/test7_success.log
else
    echo "Test 7 failed"
    echo "Test 7 failed" >&2
    cp genifai_results/test7_output.log genifai_results/test7_fail.log
    failed=1
fi
echo "Test 7 ended"

# Test 8: HTTP URL search value
echo "Test 8 started"
LD_PRELOAD=libtracer.so TRACE_OUTPUT=$PWD/genifai_results/test8_trace.log ./url-test_t8 > genifai_results/test8_output.log 2>&1
ret=$?
if [ $ret -eq 0 ]; then
    echo "Test 8 passed"
    cp genifai_results/test8_output.log genifai_results/test8_success.log
else
    echo "Test 8 failed"
    echo "Test 8 failed" >&2
    cp genifai_results/test8_output.log genifai_results/test8_fail.log
    failed=1
fi
echo "Test 8 ended"

# Test 9: HTTP URL query value
echo "Test 9 started"
LD_PRELOAD=libtracer.so TRACE_OUTPUT=$PWD/genifai_results/test9_trace.log ./url-test_t9 > genifai_results/test9_output.log 2>&1
ret=$?
if [ $ret -eq 0 ]; then
    echo "Test 9 passed"
    cp genifai_results/test9_output.log genifai_results/test9_success.log
else
    echo "Test 9 failed"
    echo "Test 9 failed" >&2
    cp genifai_results/test9_output.log genifai_results/test9_fail.log
    failed=1
fi
echo "Test 9 ended"

# Test 10: HTTP URL hash value
echo "Test 10 started"
LD_PRELOAD=libtracer.so TRACE_OUTPUT=$PWD/genifai_results/test10_trace.log ./url-test_t10 > genifai_results/test10_output.log 2>&1
ret=$?
if [ $ret -eq 0 ]; then
    echo "Test 10 passed"
    cp genifai_results/test10_output.log genifai_results/test10_success.log
else
    echo "Test 10 failed"
    echo "Test 10 failed" >&2
    cp genifai_results/test10_output.log genifai_results/test10_fail.log
    failed=1
fi
echo "Test 10 ended"

# Test 11: HTTP URL port value
echo "Test 11 started"
LD_PRELOAD=libtracer.so TRACE_OUTPUT=$PWD/genifai_results/test11_trace.log ./url-test_t11 > genifai_results/test11_output.log 2>&1
ret=$?
if [ $ret -eq 0 ]; then
    echo "Test 11 passed"
    cp genifai_results/test11_output.log genifai_results/test11_success.log
else
    echo "Test 11 failed"
    echo "Test 11 failed" >&2
    cp genifai_results/test11_output.log genifai_results/test11_fail.log
    failed=1
fi
echo "Test 11 ended"

# Test 12: HTTP URL parsed struct - all fields exist
echo "Test 12 started"
LD_PRELOAD=libtracer.so TRACE_OUTPUT=$PWD/genifai_results/test12_trace.log ./url-test_t12 > genifai_results/test12_output.log 2>&1
ret=$?
if [ $ret -eq 0 ]; then
    echo "Test 12 passed"
    cp genifai_results/test12_output.log genifai_results/test12_success.log
else
    echo "Test 12 failed"
    echo "Test 12 failed" >&2
    cp genifai_results/test12_output.log genifai_results/test12_fail.log
    failed=1
fi
echo "Test 12 ended"

# Test 13: Git URL parsed struct - all fields exist
echo "Test 13 started"
LD_PRELOAD=libtracer.so TRACE_OUTPUT=$PWD/genifai_results/test13_trace.log ./url-test_t13 > genifai_results/test13_output.log 2>&1
ret=$?
if [ $ret -eq 0 ]; then
    echo "Test 13 passed"
    cp genifai_results/test13_output.log genifai_results/test13_success.log
else
    echo "Test 13 failed"
    echo "Test 13 failed" >&2
    cp genifai_results/test13_output.log genifai_results/test13_fail.log
    failed=1
fi
echo "Test 13 ended"

# Test 14: Git URL protocol value
echo "Test 14 started"
LD_PRELOAD=libtracer.so TRACE_OUTPUT=$PWD/genifai_results/test14_trace.log ./url-test_t14 > genifai_results/test14_output.log 2>&1
ret=$?
if [ $ret -eq 0 ]; then
    echo "Test 14 passed"
    cp genifai_results/test14_output.log genifai_results/test14_success.log
else
    echo "Test 14 failed"
    echo "Test 14 failed" >&2
    cp genifai_results/test14_output.log genifai_results/test14_fail.log
    failed=1
fi
echo "Test 14 ended"

# Test 15: Git URL host value
echo "Test 15 started"
LD_PRELOAD=libtracer.so TRACE_OUTPUT=$PWD/genifai_results/test15_trace.log ./url-test_t15 > genifai_results/test15_output.log 2>&1
ret=$?
if [ $ret -eq 0 ]; then
    echo "Test 15 passed"
    cp genifai_results/test15_output.log genifai_results/test15_success.log
else
    echo "Test 15 failed"
    echo "Test 15 failed" >&2
    cp genifai_results/test15_output.log genifai_results/test15_fail.log
    failed=1
fi
echo "Test 15 ended"

# Test 16: Git URL hostname value
echo "Test 16 started"
LD_PRELOAD=libtracer.so TRACE_OUTPUT=$PWD/genifai_results/test16_trace.log ./url-test_t16 > genifai_results/test16_output.log 2>&1
ret=$?
if [ $ret -eq 0 ]; then
    echo "Test 16 passed"
    cp genifai_results/test16_output.log genifai_results/test16_success.log
else
    echo "Test 16 failed"
    echo "Test 16 failed" >&2
    cp genifai_results/test16_output.log genifai_results/test16_fail.log
    failed=1
fi
echo "Test 16 ended"

# Test 17: Git URL auth value
echo "Test 17 started"
LD_PRELOAD=libtracer.so TRACE_OUTPUT=$PWD/genifai_results/test17_trace.log ./url-test_t17 > genifai_results/test17_output.log 2>&1
ret=$?
if [ $ret -eq 0 ]; then
    echo "Test 17 passed"
    cp genifai_results/test17_output.log genifai_results/test17_success.log
else
    echo "Test 17 failed"
    echo "Test 17 failed" >&2
    cp genifai_results/test17_output.log genifai_results/test17_fail.log
    failed=1
fi
echo "Test 17 ended"

# Test 18: Git URL pathname value
echo "Test 18 started"
LD_PRELOAD=libtracer.so TRACE_OUTPUT=$PWD/genifai_results/test18_trace.log ./url-test_t18 > genifai_results/test18_output.log 2>&1
ret=$?
if [ $ret -eq 0 ]; then
    echo "Test 18 passed"
    cp genifai_results/test18_output.log genifai_results/test18_success.log
else
    echo "Test 18 failed"
    echo "Test 18 failed" >&2
    cp genifai_results/test18_output.log genifai_results/test18_fail.log
    failed=1
fi
echo "Test 18 ended"

# Test 19: Git URL path value
echo "Test 19 started"
LD_PRELOAD=libtracer.so TRACE_OUTPUT=$PWD/genifai_results/test19_trace.log ./url-test_t19 > genifai_results/test19_output.log 2>&1
ret=$?
if [ $ret -eq 0 ]; then
    echo "Test 19 passed"
    cp genifai_results/test19_output.log genifai_results/test19_success.log
else
    echo "Test 19 failed"
    echo "Test 19 failed" >&2
    cp genifai_results/test19_output.log genifai_results/test19_fail.log
    failed=1
fi
echo "Test 19 ended"

# Test 20: Protocol validation - http
echo "Test 20 started"
LD_PRELOAD=libtracer.so TRACE_OUTPUT=$PWD/genifai_results/test20_trace.log ./url-test_t20 > genifai_results/test20_output.log 2>&1
ret=$?
if [ $ret -eq 0 ]; then
    echo "Test 20 passed"
    cp genifai_results/test20_output.log genifai_results/test20_success.log
else
    echo "Test 20 failed"
    echo "Test 20 failed" >&2
    cp genifai_results/test20_output.log genifai_results/test20_fail.log
    failed=1
fi
echo "Test 20 ended"

# Test 21: Protocol validation - https
echo "Test 21 started"
LD_PRELOAD=libtracer.so TRACE_OUTPUT=$PWD/genifai_results/test21_trace.log ./url-test_t21 > genifai_results/test21_output.log 2>&1
ret=$?
if [ $ret -eq 0 ]; then
    echo "Test 21 passed"
    cp genifai_results/test21_output.log genifai_results/test21_success.log
else
    echo "Test 21 failed"
    echo "Test 21 failed" >&2
    cp genifai_results/test21_output.log genifai_results/test21_fail.log
    failed=1
fi
echo "Test 21 ended"

# Test 22: Protocol validation - git
echo "Test 22 started"
LD_PRELOAD=libtracer.so TRACE_OUTPUT=$PWD/genifai_results/test22_trace.log ./url-test_t22 > genifai_results/test22_output.log 2>&1
ret=$?
if [ $ret -eq 0 ]; then
    echo "Test 22 passed"
    cp genifai_results/test22_output.log genifai_results/test22_success.log
else
    echo "Test 22 failed"
    echo "Test 22 failed" >&2
    cp genifai_results/test22_output.log genifai_results/test22_fail.log
    failed=1
fi
echo "Test 22 ended"

# Test 23: Protocol validation - ssh
echo "Test 23 started"
LD_PRELOAD=libtracer.so TRACE_OUTPUT=$PWD/genifai_results/test23_trace.log ./url-test_t23 > genifai_results/test23_output.log 2>&1
ret=$?
if [ $ret -eq 0 ]; then
    echo "Test 23 passed"
    cp genifai_results/test23_output.log genifai_results/test23_success.log
else
    echo "Test 23 failed"
    echo "Test 23 failed" >&2
    cp genifai_results/test23_output.log genifai_results/test23_fail.log
    failed=1
fi
echo "Test 23 ended"

# Test 24: Protocol validation - sftp
echo "Test 24 started"
LD_PRELOAD=libtracer.so TRACE_OUTPUT=$PWD/genifai_results/test24_trace.log ./url-test_t24 > genifai_results/test24_output.log 2>&1
ret=$?
if [ $ret -eq 0 ]; then
    echo "Test 24 passed"
    cp genifai_results/test24_output.log genifai_results/test24_success.log
else
    echo "Test 24 failed"
    echo "Test 24 failed" >&2
    cp genifai_results/test24_output.log genifai_results/test24_fail.log
    failed=1
fi
echo "Test 24 ended"

# Test 25: Protocol validation - ftp
echo "Test 25 started"
LD_PRELOAD=libtracer.so TRACE_OUTPUT=$PWD/genifai_results/test25_trace.log ./url-test_t25 > genifai_results/test25_output.log 2>&1
ret=$?
if [ $ret -eq 0 ]; then
    echo "Test 25 passed"
    cp genifai_results/test25_output.log genifai_results/test25_success.log
else
    echo "Test 25 failed"
    echo "Test 25 failed" >&2
    cp genifai_results/test25_output.log genifai_results/test25_fail.log
    failed=1
fi
echo "Test 25 ended"

# Test 26: Protocol validation - javascript
echo "Test 26 started"
LD_PRELOAD=libtracer.so TRACE_OUTPUT=$PWD/genifai_results/test26_trace.log ./url-test_t26 > genifai_results/test26_output.log 2>&1
ret=$?
if [ $ret -eq 0 ]; then
    echo "Test 26 passed"
    cp genifai_results/test26_output.log genifai_results/test26_success.log
else
    echo "Test 26 failed"
    echo "Test 26 failed" >&2
    cp genifai_results/test26_output.log genifai_results/test26_fail.log
    failed=1
fi
echo "Test 26 ended"

exit $failed

