#!/bin/bash

# Expected output (trim both outputs to ignore trailing whitespace)
EXPECTED_OUTPUT="Preorder traversal of the constructed AVL tree is 
9 1 0 -1 5 2 6 10 11 
Preorder traversal after deletion of 10 
1 0 -1 9 5 2 6 11"

# Run the avl command and get the output
ACTUAL_OUTPUT=$(./avl)

# Function to remove trailing whitespace
trim_whitespace() {
    # Remove trailing spaces from each line and trailing blank lines
    echo "$1" | sed 's/[[:space:]]*$//' | sed -z 's/\n*$//'
}

# Trim both outputs
TRIMMED_EXPECTED=$(trim_whitespace "$EXPECTED_OUTPUT")
TRIMMED_ACTUAL=$(trim_whitespace "$ACTUAL_OUTPUT")

# Compare the outputs
if [ "$TRIMMED_EXPECTED" = "$TRIMMED_ACTUAL" ]; then
    echo "✅ Test successful: The output matches the expected output."
    exit 0
else
    echo "❌ Test failed: The output differs from the expected output."
    echo ""
    echo "Expected output (after trim):"
    echo "$TRIMMED_EXPECTED"
    echo ""
    echo "Actual output (after trim):"
    echo "$TRIMMED_ACTUAL"
    echo ""
    echo "Differences:"
    diff <(echo "$TRIMMED_EXPECTED") <(echo "$TRIMMED_ACTUAL") || true
    exit 1
fi