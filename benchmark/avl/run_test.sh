
#!/bin/bash

# Reformed test cases

failed=0
mkdir -p flow_results

# Test 1: AVL tree preorder traversal before and after deletion
test_num=1
echo "Test ${test_num} started"

EXPECTED_OUTPUT="Preorder traversal of the constructed AVL tree is 
9 1 0 -1 5 2 6 10 11 
Preorder traversal after deletion of 10 
1 0 -1 9 5 2 6 11"

trim_whitespace() {
    echo "$1" | sed 's/[[:space:]]*$//' | sed -z 's/\n*$//'
}

ACTUAL_OUTPUT=$(LD_PRELOAD=libtracer.so TRACE_OUTPUT=$PWD/flow_results/test${test_num}_trace.log ./test_avl_t1)

TRIMMED_EXPECTED=$(trim_whitespace "$EXPECTED_OUTPUT")
TRIMMED_ACTUAL=$(trim_whitespace "$ACTUAL_OUTPUT")

if [ "$TRIMMED_EXPECTED" = "$TRIMMED_ACTUAL" ]; then
    {
        echo "✅ Test successful: The output matches the expected output."
        echo "Expected output (after trim):"
        echo "$TRIMMED_EXPECTED"
        echo "Actual output (after trim):"
        echo "$TRIMMED_ACTUAL"
    } > flow_results/test${test_num}_success.log
    echo "Test ${test_num} passed"
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
    } > flow_results/test${test_num}_fail.log
    echo "Test ${test_num} failed" >&2
    failed=1
fi
echo "Test ${test_num} ended"

exit $failed

