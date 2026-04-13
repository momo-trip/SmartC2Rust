
#!/bin/bash

# Reformed test cases
failed=0

mkdir -p genifai_results

EXPECTED_OUTPUT="Preorder traversal of the constructed AVL tree is 
9 1 0 -1 5 2 6 10 11 
Preorder traversal after deletion of 10 
1 0 -1 9 5 2 6 11 "

# Test 1: Run AVL tree program and verify output
echo "Test 1 started"
actual_output=$(LD_PRELOAD=libtracer.so TRACE_OUTPUT=$PWD/genifai_results/test1_trace.log ./avl_t1 2>&1)
test1_exit=$?

if [ $test1_exit -eq 0 ] && [ "$actual_output" = "$EXPECTED_OUTPUT" ]; then
    echo "Test 1 passed"
    echo "Test 1 passed" > genifai_results/test1_success.log
    echo "Exit code: $test1_exit" >> genifai_results/test1_success.log
    echo "Output:" >> genifai_results/test1_success.log
    echo "$actual_output" >> genifai_results/test1_success.log
else
    echo "Test 1 failed"
    echo "Test 1 failed" >&2
    failed=1
    echo "Test 1 failed" > genifai_results/test1_fail.log
    echo "Exit code: $test1_exit" >> genifai_results/test1_fail.log
    echo "Expected output:" >> genifai_results/test1_fail.log
    echo "$EXPECTED_OUTPUT" >> genifai_results/test1_fail.log
    echo "Actual output:" >> genifai_results/test1_fail.log
    echo "$actual_output" >> genifai_results/test1_fail.log
fi
echo "Test 1 ended"

exit $failed

