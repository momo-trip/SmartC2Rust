#!/bin/bash

# Create a results directory
RESULTS_DIR="test-results"
rm -rf $RESULTS_DIR
mkdir -p $RESULTS_DIR

# Log files
LOG_FILE="$RESULTS_DIR/buffer_test.log"
C_LOG_FILE="/home/ubuntu/portable/out_flow_c.log"
RUST_LOG_FILE="/home/ubuntu/portable/out_flow_rust.log"

# Expected values directory
EXPECTED_DIR="expected"

# Timestamp for the test run
TIMESTAMP=$(date "+%Y-%m-%d %H:%M:%S")

# Print a divider
print_divider() {
    echo "----------------------------------------" | tee -a $LOG_FILE
}

# Log start of a test
logstart() {
    local test_num=$1
    local test_name=$2
    
    echo "Running test #${test_num}: ${test_name}" | tee -a $LOG_FILE
    echo "Test Case #${test_num}: Started" >> $C_LOG_FILE
    echo "Test Case #${test_num}: Started" >> $RUST_LOG_FILE
}

# Compare test output with expected values
compare_with_expected() {
    local test_num=$1
    local actual_output="$2"
    local expected_file="$EXPECTED_DIR/expected_values_$test_num.txt"
    
    if [ ! -f "$expected_file" ]; then
        echo "⚠️  Expected values file not found: $expected_file" | tee -a $LOG_FILE
        return 1
    fi
    
    # Read expected output
    local expected_output=$(cat "$expected_file")
    
    # Compare outputs
    if [ "$actual_output" = "$expected_output" ]; then
        echo "✅ Output matches expected values" | tee -a $LOG_FILE
        return 0
    else
        echo "❌ Output differs from expected values" | tee -a $LOG_FILE
        echo "Expected:" | tee -a $LOG_FILE
        echo "$expected_output" | tee -a $LOG_FILE
        echo "Actual:" | tee -a $LOG_FILE
        echo "$actual_output" | tee -a $LOG_FILE
        
        # Save diff to file
        local diff_file="$RESULTS_DIR/diff_test_$test_num.txt"
        echo "=== EXPECTED ===" > "$diff_file"
        echo "$expected_output" >> "$diff_file"
        echo "=== ACTUAL ===" >> "$diff_file"
        echo "$actual_output" >> "$diff_file"
        echo "=== DIFF ===" >> "$diff_file"
        diff <(echo "$expected_output") <(echo "$actual_output") >> "$diff_file" 2>&1
        
        echo "Detailed comparison saved to: $diff_file" | tee -a $LOG_FILE
        return 1
    fi
}

print_divider

# Record the start time
START_TIME=$(date +%s)

# Initialize counters
TOTAL_TESTS=0
PASSED_TESTS=0
FAILED_TESTS=0

# Loop through test numbers 1 to 17
for ((current_test=1; current_test<=17; current_test++)); do
    test_name="Buffer Test $current_test"
    
    # Log test start
    logstart $current_test "$test_name"
    
    # Run the test and capture both stdout and stderr
    echo "Running buffer test $current_test..." | tee -a $LOG_FILE
    TEST_OUTPUT=$(./test $current_test 2>&1)
    TEST_RESULT=$?
    
    # Record the end time and calculate duration
    END_TIME=$(date +%s)
    DURATION=$((END_TIME - START_TIME))
    
    # Save the complete test output
    echo "$TEST_OUTPUT" | tee "$RESULTS_DIR/test_output_$current_test.txt"
    
    # Increment total tests counter
    TOTAL_TESTS=$((TOTAL_TESTS + 1))
    
    # Display the test result
    print_divider
    
    # Check if test execution was successful: Now compare with expected values
    if compare_with_expected $current_test "$TEST_OUTPUT"; then
        PASSED_TESTS=$((PASSED_TESTS + 1))
    else
        FAILED_TESTS=$((FAILED_TESTS + 1))
    fi
    
    print_divider
    echo "Detailed test output saved to: $RESULTS_DIR/test_output_$current_test.txt" | tee -a $LOG_FILE
    echo "Log file: $LOG_FILE" | tee -a $LOG_FILE
    print_divider
done

# Print summary
echo "" | tee -a $LOG_FILE
echo "=== TEST SUMMARY ===" | tee -a $LOG_FILE
echo "Total tests run: $TOTAL_TESTS" | tee -a $LOG_FILE
echo "Tests passed: $PASSED_TESTS" | tee -a $LOG_FILE
echo "Tests failed: $FAILED_TESTS" | tee -a $LOG_FILE

if [ $FAILED_TESTS -eq 0 ]; then
    echo -e "\e[32m🎉 ALL TESTS PASSED!\e[0m" | tee -a $LOG_FILE
    exit 0
else
    echo -e "\e[31m❌ $FAILED_TESTS test(s) failed\e[0m" | tee -a $LOG_FILE
    exit 1
fi