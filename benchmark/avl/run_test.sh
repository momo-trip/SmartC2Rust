
#!/bin/bash

# Reformed test cases

failed=0
mkdir -p flow_results

# Expected output
EXPECTED_OUTPUT="Preorder traversal of the constructed AVL tree is 
9 1 0 -1 5 2 6 10 11 
Preorder traversal after deletion of 10 
1 0 -1 9 5 2 6 11"

trim_whitespace() {
    echo "$1" | sed 's/[[:space:]]*$//' | sed -z 's/\n*$//'
}

# Test 1: Verify AVL tree preorder traversal output matches expected
echo "Test 1 started"

TRIMMED_EXPECTED=$(trim_whitespace "$EXPECTED_OUTPUT")

ACTUAL_OUTPUT=$(LD_PRELOAD=libtracer.so TRACE_OUTPUT=$PWD/flow_results/test1_trace.log ./test_avl_t1)
exit_code=$?

TRIMMED_ACTUAL=$(trim_whitespace "$ACTUAL_OUTPUT")

{
    echo "Exit code: $exit_code"
    echo "Expected output (after trim):"
    echo "$TRIMMED_EXPECTED"
    echo ""
    echo "Actual output (after trim):"
    echo "$TRIMMED_ACTUAL"
    echo ""
    echo "Differences:"
    diff <(echo "$TRIMMED_EXPECTED") <(echo "$TRIMMED_ACTUAL")
} > /tmp/test1_output.log 2>&1

if [ "$TRIMMED_EXPECTED" = "$TRIMMED_ACTUAL" ] && [ $exit_code -eq 0 ]; then
    cp /tmp/test1_output.log flow_results/test1_success.log
    echo "Test 1 passed"
else
    cp /tmp/test1_output.log flow_results/test1_fail.log
    echo "Test 1 failed" >&2
    failed=1
fi

echo "Test 1 ended"

exit $failed

