
#!/bin/bash

# Reformed test cases

mkdir -p flow_results
failed=0

# ----------------- Test 1: Named colors -----------------
echo "Test 1 started"
LOG1=$(LD_PRELOAD=libtracer.so TRACE_OUTPUT=$PWD/flow_results/test1_trace.log ./test_rgba_t1 1 2>&1)
RET1=$?
if [ $RET1 -eq 0 ]; then
    echo "$LOG1" > flow_results/test1_success.log
    echo "Test 1 passed"
else
    echo "$LOG1" > flow_results/test1_fail.log
    echo "Test 1 failed" >&2
    failed=1
fi
echo "Test 1 ended"

# ----------------- Test 2: RGB format -----------------
echo "Test 2 started"
LOG2=$(LD_PRELOAD=libtracer.so TRACE_OUTPUT=$PWD/flow_results/test2_trace.log ./test_rgba_t2 2 2>&1)
RET2=$?
if [ $RET2 -eq 0 ]; then
    echo "$LOG2" > flow_results/test2_success.log
    echo "Test 2 passed"
else
    echo "$LOG2" > flow_results/test2_fail.log
    echo "Test 2 failed" >&2
    failed=1
fi
echo "Test 2 ended"

# ----------------- Test 3: RGBA format -----------------
echo "Test 3 started"
LOG3=$(LD_PRELOAD=libtracer.so TRACE_OUTPUT=$PWD/flow_results/test3_trace.log ./test_rgba_t3 3 2>&1)
RET3=$?
if [ $RET3 -eq 0 ]; then
    echo "$LOG3" > flow_results/test3_success.log
    echo "Test 3 passed"
else
    echo "$LOG3" > flow_results/test3_fail.log
    echo "Test 3 failed" >&2
    failed=1
fi
echo "Test 3 ended"

# ----------------- Test 4: Hex format -----------------
echo "Test 4 started"
LOG4=$(LD_PRELOAD=libtracer.so TRACE_OUTPUT=$PWD/flow_results/test4_trace.log ./test_rgba_t4 4 2>&1)
RET4=$?
if [ $RET4 -eq 0 ]; then
    echo "$LOG4" > flow_results/test4_success.log
    echo "Test 4 passed"
else
    echo "$LOG4" > flow_results/test4_fail.log
    echo "Test 4 failed" >&2
    failed=1
fi
echo "Test 4 ended"

# ----------------- Test 5: To string conversion -----------------
echo "Test 5 started"
LOG5=$(LD_PRELOAD=libtracer.so TRACE_OUTPUT=$PWD/flow_results/test5_trace.log ./test_rgba_t5 5 2>&1)
RET5=$?
if [ $RET5 -eq 0 ]; then
    echo "$LOG5" > flow_results/test5_success.log
    echo "Test 5 passed"
else
    echo "$LOG5" > flow_results/test5_fail.log
    echo "Test 5 failed" >&2
    failed=1
fi
echo "Test 5 ended"

exit $failed

