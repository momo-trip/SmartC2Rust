#!/bin/bash

# Get the test number from the command line argument
for ((current_test=1; current_test<=17; current_test++)); do
    TEST_NUM=$current_test

    # Run the test and capture both stdout and stderr
    echo "Running buffer test $TEST_NUM to get expected values..."
    TEST_OUTPUT=$(./test $TEST_NUM 2>&1)

    # Extract the expected values using grep
    EXPECTED_VALUES=$(echo "$TEST_OUTPUT")

    # Create a subdirectory for the test number
    mkdir -p "expected"

    # Save the expected values to a file in the test subdirectory
    echo "$EXPECTED_VALUES" > "expected/expected_values_$TEST_NUM.txt"

    echo "Expected values for test $TEST_NUM saved to expected/expected_values_$TEST_NUM.txt"

done