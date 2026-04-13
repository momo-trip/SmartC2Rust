
#!/bin/bash

# Reformed test cases
failed=0
mkdir -p flow_results

##############################################
# Test 1: Named colors (test_named assertion)
##############################################
echo "Test 1 started"
log=""
ACTUAL_OUTPUT=$(LD_PRELOAD=libtracer.so TRACE_OUTPUT=$PWD/flow_results/test1_trace.log ./test_rgba_t1 2>&1)
test1_exit=$?
log+="Exit code: ${test1_exit}\n"
log+="Output:\n${ACTUAL_OUTPUT}\n"

if [ $test1_exit -eq 0 ]; then
    echo "Test 1 passed"
    echo -e "$log" > flow_results/test1_success.log
else
    echo "Test 1 failed"
    echo "Test 1 failed" >&2
    echo -e "$log" > flow_results/test1_fail.log
    failed=1
fi
echo "Test 1 ended"

##############################################
# Test 2: RGB format (test_rgb assertion)
##############################################
echo "Test 2 started"
log=""
ACTUAL_OUTPUT=$(LD_PRELOAD=libtracer.so TRACE_OUTPUT=$PWD/flow_results/test2_trace.log ./test_rgba_t2 2>&1)
test2_exit=$?
log+="Exit code: ${test2_exit}\n"
log+="Output:\n${ACTUAL_OUTPUT}\n"

if [ $test2_exit -eq 0 ]; then
    echo "Test 2 passed"
    echo -e "$log" > flow_results/test2_success.log
else
    echo "Test 2 failed"
    echo "Test 2 failed" >&2
    echo -e "$log" > flow_results/test2_fail.log
    failed=1
fi
echo "Test 2 ended"

##############################################
# Test 3: RGBA format (test_rgba assertion)
##############################################
echo "Test 3 started"
log=""
ACTUAL_OUTPUT=$(LD_PRELOAD=libtracer.so TRACE_OUTPUT=$PWD/flow_results/test3_trace.log ./test_rgba_t3 2>&1)
test3_exit=$?
log+="Exit code: ${test3_exit}\n"
log+="Output:\n${ACTUAL_OUTPUT}\n"

if [ $test3_exit -eq 0 ]; then
    echo "Test 3 passed"
    echo -e "$log" > flow_results/test3_success.log
else
    echo "Test 3 failed"
    echo "Test 3 failed" >&2
    echo -e "$log" > flow_results/test3_fail.log
    failed=1
fi
echo "Test 3 ended"

##############################################
# Test 4: Hex format (test_hex assertion)
##############################################
echo "Test 4 started"
log=""
ACTUAL_OUTPUT=$(LD_PRELOAD=libtracer.so TRACE_OUTPUT=$PWD/flow_results/test4_trace.log ./test_rgba_t4 2>&1)
test4_exit=$?
log+="Exit code: ${test4_exit}\n"
log+="Output:\n${ACTUAL_OUTPUT}\n"

if [ $test4_exit -eq 0 ]; then
    echo "Test 4 passed"
    echo -e "$log" > flow_results/test4_success.log
else
    echo "Test 4 failed"
    echo "Test 4 failed" >&2
    echo -e "$log" > flow_results/test4_fail.log
    failed=1
fi
echo "Test 4 ended"

##############################################
# Test 5: To string conversion (test_to_string assertion)
##############################################
echo "Test 5 started"
log=""
ACTUAL_OUTPUT=$(LD_PRELOAD=libtracer.so TRACE_OUTPUT=$PWD/flow_results/test5_trace.log ./test_rgba_t5 2>&1)
test5_exit=$?
log+="Exit code: ${test5_exit}\n"
log+="Output:\n${ACTUAL_OUTPUT}\n"

if [ $test5_exit -eq 0 ]; then
    echo "Test 5 passed"
    echo -e "$log" > flow_results/test5_success.log
else
    echo "Test 5 failed"
    echo "Test 5 failed" >&2
    echo -e "$log" > flow_results/test5_fail.log
    failed=1
fi
echo "Test 5 ended"

exit $failed

