#!/bin/bash

# Script to capture expected results from su-exec test cases
# This script runs each test case and saves the expected output/exit code

EXPECTED_DIR="expected"
rm -rf "$EXPECTED_DIR"
mkdir -p "$EXPECTED_DIR"

echo "=== Capturing expected results for su-exec test cases ==="

# Function to capture test result
capture_test() {
    local test_name="$1"
    local command="$2"
    local description="$3"
    
    echo "Capturing: $test_name - $description"
    
    # Capture stdout, stderr, and exit code
    local stdout_file="$EXPECTED_DIR/${test_name}_stdout.txt"
    local stderr_file="$EXPECTED_DIR/${test_name}_stderr.txt"
    local exitcode_file="$EXPECTED_DIR/${test_name}_exitcode.txt"
    
    # Execute command and capture all outputs
    eval "$command" > "$stdout_file" 2> "$stderr_file"
    echo "$?" > "$exitcode_file"
    
    # Show summary
    local exit_code=$(cat "$exitcode_file")
    local stdout_lines=$(wc -l < "$stdout_file")
    local stderr_lines=$(wc -l < "$stderr_file")
    
    echo "  Exit code: $exit_code, Stdout lines: $stdout_lines, Stderr lines: $stderr_lines"
}

# Test case 1: No arguments (should show usage)
capture_test "test01_no_args" \
    "./su-exec" \
    "Execute su-exec without any arguments"

# Test case 2: One argument (should show usage)
capture_test "test02_one_arg" \
    "./su-exec user1" \
    "Execute su-exec with only one argument"

# Test case 3: Help flag (should show usage)
capture_test "test03_help_flag" \
    "./su-exec --help" \
    "Execute su-exec with the help flag"

# Test case 4: Invalid option (should show usage)
capture_test "test04_invalid_option" \
    "./su-exec -h" \
    "Execute su-exec with invalid options"

# Test case 5: Basic execution with root user
capture_test "test05_basic_root" \
    "./su-exec root echo 'Hello World'" \
    "Execute su-exec with minimum required arguments"

# Test case 6: User:group specification
capture_test "test06_user_group" \
    "./su-exec user:group echo 'test'" \
    "Test with user:group specification"

# Test case 7: Numeric UID specification
capture_test "test07_numeric_uid" \
    "./su-exec 1000 echo 'test'" \
    "Test with numeric UID specification"

# Test case 8: Numeric GID specification
capture_test "test08_numeric_gid" \
    "./su-exec user:1000 echo 'test'" \
    "Test with numeric GID specification"

# Test case 9: Non-existent user
capture_test "test09_nonexistent_user" \
    "./su-exec nonexistent_user echo 'test'" \
    "Test with non-existent user"

# Test case 10: Non-existent group
capture_test "test10_nonexistent_group" \
    "./su-exec user:nonexistent_group echo 'test'" \
    "Test with non-existent group"

# Test case 11: Valid user:group combination
capture_test "test11_valid_user_group" \
    "./su-exec root:root echo 'test'" \
    "Test with valid user:group combination"

# Test case 12: Numeric GID with root
capture_test "test12_numeric_gid_root" \
    "./su-exec root:0 echo 'test'" \
    "Test with numeric GID specification (covers lines 66-68)"

# Test case 13: Empty username
capture_test "test13_empty_username" \
    "./su-exec '' echo 'test'" \
    "Test with empty username to create pw=NULL state"

# Test case 14: Numeric UID only
capture_test "test14_numeric_uid_only" \
    "./su-exec 0 echo 'test'" \
    "Test with numeric UID only to create pw=NULL state"

# Test case 15: Large numeric UID
capture_test "test15_large_numeric_uid" \
    "./su-exec 65534 echo 'test'" \
    "Test with large numeric UID (nobody user)"

# Test case 16: Invalid numeric UID (too large)
capture_test "test16_invalid_large_uid" \
    "./su-exec 4294967296 echo 'test'" \
    "Test with invalid large numeric UID"

# Test case 17: Negative numeric UID
capture_test "test17_negative_uid" \
    "./su-exec -1 echo 'test'" \
    "Test with negative numeric UID"

# Test case 18: User with colon but no group
capture_test "test18_user_colon_no_group" \
    "./su-exec user: echo 'test'" \
    "Test with user followed by colon but no group"

# Test case 19: Only colon (empty user and group)
capture_test "test19_only_colon" \
    "./su-exec : echo 'test'" \
    "Test with only colon (empty user and group)"

# Test case 20: Multiple colons in user spec
capture_test "test20_multiple_colons" \
    "./su-exec user:group:extra echo 'test'" \
    "Test with multiple colons in user specification"

echo ""
echo "=== Expected results captured in $EXPECTED_DIR ==="
echo ""

# Create a summary file
SUMMARY_FILE="$EXPECTED_DIR/summary.txt"
echo "Su-exec Test Results Summary" > "$SUMMARY_FILE"
echo "Generated on: $(date)" >> "$SUMMARY_FILE"
echo "=================================" >> "$SUMMARY_FILE"
echo "" >> "$SUMMARY_FILE"

for test_dir in "$EXPECTED_DIR"/test*_exitcode.txt; do
    if [ -f "$test_dir" ]; then
        test_name=$(basename "$test_dir" _exitcode.txt)
        exit_code=$(cat "$test_dir")
        stdout_lines=$(wc -l < "$EXPECTED_DIR/${test_name}_stdout.txt" 2>/dev/null || echo "0")
        stderr_lines=$(wc -l < "$EXPECTED_DIR/${test_name}_stderr.txt" 2>/dev/null || echo "0")
        
        echo "$test_name: Exit=$exit_code, Stdout=$stdout_lines lines, Stderr=$stderr_lines lines" >> "$SUMMARY_FILE"
    fi
done

echo "Summary written to: $SUMMARY_FILE"