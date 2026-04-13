
#!/bin/bash

failed=0
mkdir -p flow_results

# Function to remove trailing whitespace
trim_whitespace() {
    echo "$1" | sed 's/[[:space:]]*$//' | sed -z 's/\n*$//'
}

##############################################
# Test 1: Run bst and compare output
##############################################
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

ACTUAL_OUTPUT=$(LD_PRELOAD=libtracer.so TRACE_OUTPUT=$PWD/flow_results/test1_trace.log ./bst_t1)
run_status=$?

TRIMMED_EXPECTED=$(trim_whitespace "$EXPECTED_OUTPUT")
TRIMMED_ACTUAL=$(trim_whitespace "$ACTUAL_OUTPUT")

if [ $run_status -eq 0 ] && [ "$TRIMMED_EXPECTED" = "$TRIMMED_ACTUAL" ]; then
    echo "Test 1 passed"
    {
        echo "Test 1 passed"
        echo "Expected output:"
        echo "$TRIMMED_EXPECTED"
        echo "Actual output:"
        echo "$TRIMMED_ACTUAL"
    } > flow_results/test1_success.log
else
    echo "Test 1 failed"
    echo "Test 1 failed" >&2
    failed=1
    {
        echo "Test 1 failed"
        echo "Exit status: $run_status"
        echo "Expected output (after trim):"
        echo "$TRIMMED_EXPECTED"
        echo ""
        echo "Actual output (after trim):"
        echo "$TRIMMED_ACTUAL"
        echo ""
        echo "Differences:"
        diff <(echo "$TRIMMED_EXPECTED") <(echo "$TRIMMED_ACTUAL")
    } > flow_results/test1_fail.log 2>&1
fi

echo "Test 1 ended"

exit $failed

