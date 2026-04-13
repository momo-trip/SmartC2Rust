
#!/bin/bash

failed=0
mkdir -p genifai_results

# Test 1
echo "Test 1 started"
output=$(LD_PRELOAD=libtracer.so TRACE_OUTPUT=$PWD/genifai_results/test1_trace.log ./markandsweep_t1 1 2>&1)
test1_status=$?
if [ $test1_status -eq 0 ]; then
    echo "$output" > genifai_results/test1_success.log
    echo "Test 1 passed"
else
    echo "$output" > genifai_results/test1_fail.log
    echo "Test 1 failed"
    echo "Test 1 failed" >&2
    failed=1
fi
echo "Test 1 ended"

# Test 2
echo "Test 2 started"
output=$(LD_PRELOAD=libtracer.so TRACE_OUTPUT=$PWD/genifai_results/test2_trace.log ./markandsweep_t2 2 2>&1)
test2_status=$?
if [ $test2_status -eq 0 ]; then
    echo "$output" > genifai_results/test2_success.log
    echo "Test 2 passed"
else
    echo "$output" > genifai_results/test2_fail.log
    echo "Test 2 failed"
    echo "Test 2 failed" >&2
    failed=1
fi
echo "Test 2 ended"

# Test 3
echo "Test 3 started"
output=$(LD_PRELOAD=libtracer.so TRACE_OUTPUT=$PWD/genifai_results/test3_trace.log ./markandsweep_t3 3 2>&1)
test3_status=$?
if [ $test3_status -eq 0 ]; then
    echo "$output" > genifai_results/test3_success.log
    echo "Test 3 passed"
else
    echo "$output" > genifai_results/test3_fail.log
    echo "Test 3 failed"
    echo "Test 3 failed" >&2
    failed=1
fi
echo "Test 3 ended"

# Test 4
echo "Test 4 started"
output=$(LD_PRELOAD=libtracer.so TRACE_OUTPUT=$PWD/genifai_results/test4_trace.log ./markandsweep_t4 4 2>&1)
test4_status=$?
if [ $test4_status -eq 0 ]; then
    echo "$output" > genifai_results/test4_success.log
    echo "Test 4 passed"
else
    echo "$output" > genifai_results/test4_fail.log
    echo "Test 4 failed"
    echo "Test 4 failed" >&2
    failed=1
fi
echo "Test 4 ended"

# Test 5
echo "Test 5 started"
output=$(LD_PRELOAD=libtracer.so TRACE_OUTPUT=$PWD/genifai_results/test5_trace.log ./markandsweep_t5 5 2>&1)
test5_status=$?
if [ $test5_status -eq 0 ]; then
    echo "$output" > genifai_results/test5_success.log
    echo "Test 5 passed"
else
    echo "$output" > genifai_results/test5_fail.log
    echo "Test 5 failed"
    echo "Test 5 failed" >&2
    failed=1
fi
echo "Test 5 ended"

exit $failed

