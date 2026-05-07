
#!/bin/bash

# Reformed test cases

failed=0
mkdir -p flow_results

# Test 1: Verify full inorder traversal and deletion behavior of BST
echo "Test 1 started"

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

ACTUAL_OUTPUT=$(LD_PRELOAD=libtracer.so TRACE_OUTPUT=$PWD/flow_results/test1_trace.log ./test_bst_t1)

trim_whitespace() {
    echo "$1" | sed 's/[[:space:]]*$//' | sed -z 's/\n*$//'
}

TRIMMED_EXPECTED=$(trim_whitespace "$EXPECTED_OUTPUT")
TRIMMED_ACTUAL=$(trim_whitespace "$ACTUAL_OUTPUT")

if [ "$TRIMMED_EXPECTED" = "$TRIMMED_ACTUAL" ]; then
    {
        echo "✅ Test successful: The output matches the expected output."
        echo "Actual output:"
        echo "$TRIMMED_ACTUAL"
    } > flow_results/test1_success.log
    echo "Test 1 passed"
else
    {
        echo "❌ Test failed: The output differs from the expected output."
        echo ""
        echo "Expected output (after trim):"
        echo "$TRIMMED_EXPECTED"
        echo ""
        echo "Actual output (after trim):"
        echo "$TRIMMED_ACTUAL"
        echo ""
        echo "Differences:"
        diff <(echo "$TRIMMED_EXPECTED") <(echo "$TRIMMED_ACTUAL")
    } > flow_results/test1_fail.log
    echo "Test 1 failed" >&2
    failed=1
fi

echo "Test 1 ended"

exit $failed

