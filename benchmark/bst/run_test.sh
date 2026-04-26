
#!/bin/bash

# Reformed test cases

failed=0
mkdir -p flow_results

# Expected output
EXPECTED_OUTPUT="Inorder traversal of the given tree 
20 30 40 50 60 70 80 
Delete 20
Inorder traversal of the modified tree 
30 40 50 60 70 80 
Delete 30
Inorder traversal of the modified tree 
40 50 60 70 80 
Delete 50
Inorder traversal of the modified tree 
40 60 70 80"

trim_whitespace() {
    echo "$1" | sed 's/[[:space:]]*$//' | sed -z 's/\n*$//'
}

# Test 1: Verify full output of BST operations
echo "Test 1 started"

TRIMMED_EXPECTED=$(trim_whitespace "$EXPECTED_OUTPUT")
ACTUAL_OUTPUT=$(LD_PRELOAD=libtracer.so TRACE_OUTPUT=$PWD/flow_results/test1_trace.log ./test_bst_t1)
rc=$?
TRIMMED_ACTUAL=$(trim_whitespace "$ACTUAL_OUTPUT")

{
    echo "Return code: $rc"
    echo "Expected output (after trim):"
    echo "$TRIMMED_EXPECTED"
    echo ""
    echo "Actual output (after trim):"
    echo "$TRIMMED_ACTUAL"
    echo ""
    echo "Differences:"
    diff <(echo "$TRIMMED_EXPECTED") <(echo "$TRIMMED_ACTUAL")
} > /tmp/test1_output.log 2>&1

if [ $rc -eq 0 ] && [ "$TRIMMED_EXPECTED" = "$TRIMMED_ACTUAL" ]; then
    cp /tmp/test1_output.log flow_results/test1_success.log
    echo "Test 1 passed"
else
    cp /tmp/test1_output.log flow_results/test1_fail.log
    echo "Test 1 failed" >&2
    failed=1
fi

echo "Test 1 ended"

rm -f /tmp/test1_output.log

exit $failed

