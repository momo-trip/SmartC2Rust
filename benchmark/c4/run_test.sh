
#!/bin/bash

# Reformed test cases
failed=0
mkdir -p flow_results

# Test 1: Run hello.c through c4 interpreter
echo "Test 1 started"
log1=$(LD_PRELOAD=libtracer.so TRACE_OUTPUT=$PWD/flow_results/test1_trace.log ./c4_t1 hello.c 2>&1)
ret1=$?
if [ $ret1 -eq 0 ] && echo "$log1" | grep -q "hello, world"; then
    echo "$log1" > flow_results/test1_success.log
    echo "Test 1 passed"
else
    echo "$log1" > flow_results/test1_fail.log
    echo "Test 1 failed"
    echo "Test 1 failed" >&2
    failed=1
fi
echo "Test 1 ended"

# Test 2: Run hello.c through c4 interpreting itself (self-hosting)
echo "Test 2 started"
log2=$(LD_PRELOAD=libtracer.so TRACE_OUTPUT=$PWD/flow_results/test2_trace.log ./c4_t2 c4.c hello.c 2>&1)
ret2=$?
if [ $ret2 -eq 0 ] && echo "$log2" | grep -q "hello, world"; then
    echo "$log2" > flow_results/test2_success.log
    echo "Test 2 passed"
else
    echo "$log2" > flow_results/test2_fail.log
    echo "Test 2 failed"
    echo "Test 2 failed" >&2
    failed=1
fi
echo "Test 2 ended"

exit $failed

failed=0
mkdir -p flow_results

# Test 1: Run hello.c through c4 interpreter
echo "Test 1 started"
log1=$(LD_PRELOAD=libtracer.so TRACE_OUTPUT=$PWD/flow_results/test1_trace.log ./c4_t1 hello.c 2>&1)
ret1=$?
if [ $ret1 -eq 0 ] && echo "$log1" | grep -q "hello, world"; then
    echo "$log1" > flow_results/test1_success.log
    echo "Test 1 passed"
else
    echo "$log1" > flow_results/test1_fail.log
    echo "Test 1 failed"
    echo "Test 1 failed" >&2
    failed=1
fi
echo "Test 1 ended"

# Test 2: Run hello.c through c4 interpreting itself (self-hosting)
echo "Test 2 started"
log2=$(LD_PRELOAD=libtracer.so TRACE_OUTPUT=$PWD/flow_results/test2_trace.log ./c4_t2 c4.c hello.c 2>&1)
ret2=$?
if [ $ret2 -eq 0 ] && echo "$log2" | grep -q "hello, world"; then
    echo "$log2" > flow_results/test2_success.log
    echo "Test 2 passed"
else
    echo "$log2" > flow_results/test2_fail.log
    echo "Test 2 failed"
    echo "Test 2 failed" >&2
    failed=1
fi
echo "Test 2 ended"

exit $failed

