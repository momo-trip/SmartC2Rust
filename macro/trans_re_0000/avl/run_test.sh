
#!/bin/bash

# Reformed test cases

failed=0

mkdir -p flow_results

EXPECTED_OUTPUT="Preorder traversal of the constructed AVL tree is 
9 1 0 -1 5 2 6 10 11 
Preorder traversal after deletion of 10 
1 0 -1 9 5 2 6 11 "

# Test 1: Run avl_t1 and verify output
echo "Test 1 started"
actual_output=$(LD_PRELOAD=libtracer.so TRACE_OUTPUT=$PWD/flow_results/test1_trace.log ./avl_t1 2>&1)
test1_exit=$?
if [ $test1_exit -eq 0 ] && [ "$actual_output" = "$EXPECTED_OUTPUT" ]; then
    echo "Test 1 passed"
    echo "$actual_output" > flow_results/test1_success.log
else
    echo "Test 1 failed"
    echo "Test 1 failed" >&2
    echo "Exit code: $test1_exit" > flow_results/test1_fail.log
    echo "Expected output:" >> flow_results/test1_fail.log
    echo "$EXPECTED_OUTPUT" >> flow_results/test1_fail.log
    echo "Actual output:" >> flow_results/test1_fail.log
    echo "$actual_output" >> flow_results/test1_fail.log
    failed=1
fi
echo "Test 1 ended"

exit $failed

