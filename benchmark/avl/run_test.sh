
#!/bin/bash

# Reformed test cases

failed=0

mkdir -p flow_results

# Function to remove trailing whitespace
trim_whitespace() {
    # Remove trailing spaces from each line and trailing blank lines
    echo "$1" | sed 's/[[:space:]]*$//' | sed -z 's/\n*$//'
}

# ---------------- Test 1 ----------------
echo "Test 1 started"

EXPECTED_OUTPUT="Preorder traversal of the constructed AVL tree is 
9 1 0 -1 5 2 6 10 11 
Preorder traversal after deletion of 10 
1 0 -1 9 5 2 6 11"

ACTUAL_OUTPUT=$(LD_PRELOAD=libtracer.so TRACE_OUTPUT=$PWD/flow_results/test1_trace.log ./test_avl_t1)

TRIMMED_EXPECTED=$(trim_whitespace "$EXPECTED_OUTPUT")
TRIMMED_ACTUAL=$(trim_whitespace "$ACTUAL_OUTPUT")

if [ "$TRIMMED_EXPECTED" = "$TRIMMED_ACTUAL" ]; then
    {
        echo "✅ Test successful: The output matches the expected output."
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
    echo "Test 1 failed"
    echo "Test 1 failed" >&2
    failed=1
fi

echo "Test 1 ended"

exit $failed

