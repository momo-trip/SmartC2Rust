#!/bin/bash

# Test runner script for su-exec
# Compares actual results with expected results to determine test success/failure

EXPECTED_DIR="expected"
ACTUAL_DIR="actual"
RESULTS_DIR="results"

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# Counters
TOTAL_TESTS=0
PASSED_TESTS=0
FAILED_TESTS=0

# Function to print colored output
print_result() {
    local status="$1"
    local message="$2"
    
    if [ "$status" = "PASS" ]; then
        echo -e "${GREEN}[PASS]${NC} $message"
        ((PASSED_TESTS++))
    elif [ "$status" = "FAIL" ]; then
        echo -e "${RED}[FAIL]${NC} $message"
        ((FAILED_TESTS++))
    elif [ "$status" = "WARN" ]; then
        echo -e "${YELLOW}[WARN]${NC} $message"
    else
        echo -e "$message"
    fi
}

# Function to run a single test case
run_test() {
    local test_name="$1"
    local command="$2"
    local description="$3"
    
    echo "Running: $test_name - $description"
    
    # Prepare actual output files
    local actual_stdout="$ACTUAL_DIR/${test_name}_stdout.txt"
    local actual_stderr="$ACTUAL_DIR/${test_name}_stderr.txt"
    local actual_exitcode="$ACTUAL_DIR/${test_name}_exitcode.txt"
    
    # Expected output files
    local expected_stdout="$EXPECTED_DIR/${test_name}_stdout.txt"
    local expected_stderr="$EXPECTED_DIR/${test_name}_stderr.txt"
    local expected_exitcode="$EXPECTED_DIR/${test_name}_exitcode.txt"
    
    # Execute command and capture all outputs
    eval "$command" > "$actual_stdout" 2> "$actual_stderr"
    echo "$?" > "$actual_exitcode"
    
    # Compare results
    local test_passed=true
    local differences=""
    
    # Check if expected files exist
    if [ ! -f "$expected_stdout" ] || [ ! -f "$expected_stderr" ] || [ ! -f "$expected_exitcode" ]; then
        print_result "FAIL" "$test_name: Expected files not found"
        ((TOTAL_TESTS++))
        return 1
    fi
    
    # Compare exit codes
    local expected_exit=$(cat "$expected_exitcode")
    local actual_exit=$(cat "$actual_exitcode")
    if [ "$expected_exit" != "$actual_exit" ]; then
        test_passed=false
        differences="${differences}Exit code: expected=$expected_exit, actual=$actual_exit\n"
    fi
    
    # Compare stdout
    if ! diff -q "$expected_stdout" "$actual_stdout" > /dev/null 2>&1; then
        test_passed=false
        differences="${differences}Stdout differs\n"
    fi
    
    # Compare stderr
    if ! diff -q "$expected_stderr" "$actual_stderr" > /dev/null 2>&1; then
        test_passed=false
        differences="${differences}Stderr differs\n"
    fi
    
    # Report result
    if [ "$test_passed" = true ]; then
        print_result "PASS" "$test_name"
    else
        print_result "FAIL" "$test_name"
        echo -e "  Differences:"
        echo -e "  $differences"
        
        # Save detailed diff to results directory
        local diff_file="$RESULTS_DIR/${test_name}_diff.txt"
        echo "=== Differences for $test_name ===" > "$diff_file"
        echo "Expected exit code: $expected_exit" >> "$diff_file"
        echo "Actual exit code: $actual_exit" >> "$diff_file"
        echo "" >> "$diff_file"
        echo "=== Stdout diff ===" >> "$diff_file"
        diff "$expected_stdout" "$actual_stdout" >> "$diff_file" 2>&1
        echo "" >> "$diff_file"
        echo "=== Stderr diff ===" >> "$diff_file"
        diff "$expected_stderr" "$actual_stderr" >> "$diff_file" 2>&1
    fi
    
    ((TOTAL_TESTS++))
}


echo "=== Su-exec Test Runner ==="
echo "Starting test execution at $(date)"
echo ""

# Check if su-exec binary exists
if [ ! -f "./su-exec" ]; then
    echo -e "${RED}Error: su-exec binary not found in current directory${NC}"
    exit 1
fi

# Check if expected directory exists
if [ ! -d "$EXPECTED_DIR" ]; then
    echo -e "${RED}Error: Expected results directory '$EXPECTED_DIR' not found${NC}"
    echo "Please run the capture script first to generate expected results"
    exit 1
fi

# Create directories for actual results and diffs
rm -rf "$ACTUAL_DIR" "$RESULTS_DIR"
mkdir -p "$ACTUAL_DIR" "$RESULTS_DIR"

# Run all test cases (same as in the capture script)
run_test "test01_no_args" \
    "./su-exec" \
    "Execute su-exec without any arguments"

run_test "test02_one_arg" \
    "./su-exec user1" \
    "Execute su-exec with only one argument"

run_test "test03_help_flag" \
    "./su-exec --help" \
    "Execute su-exec with the help flag"

run_test "test04_invalid_option" \
    "./su-exec -h" \
    "Execute su-exec with invalid options"

run_test "test05_basic_root" \
    "./su-exec root echo 'Hello World'" \
    "Execute su-exec with minimum required arguments"

run_test "test06_user_group" \
    "./su-exec user:group echo 'test'" \
    "Test with user:group specification"

run_test "test07_numeric_uid" \
    "./su-exec 1000 echo 'test'" \
    "Test with numeric UID specification"

run_test "test08_numeric_gid" \
    "./su-exec user:1000 echo 'test'" \
    "Test with numeric GID specification"

run_test "test09_nonexistent_user" \
    "./su-exec nonexistent_user echo 'test'" \
    "Test with non-existent user"

run_test "test10_nonexistent_group" \
    "./su-exec user:nonexistent_group echo 'test'" \
    "Test with non-existent group"

run_test "test11_valid_user_group" \
    "./su-exec root:root echo 'test'" \
    "Test with valid user:group combination"

run_test "test12_numeric_gid_root" \
    "./su-exec root:0 echo 'test'" \
    "Test with numeric GID specification (covers lines 66-68)"

run_test "test13_empty_username" \
    "./su-exec '' echo 'test'" \
    "Test with empty username to create pw=NULL state"

run_test "test14_numeric_uid_only" \
    "./su-exec 0 echo 'test'" \
    "Test with numeric UID only to create pw=NULL state"

run_test "test15_large_numeric_uid" \
    "./su-exec 65534 echo 'test'" \
    "Test with large numeric UID (nobody user)"

run_test "test16_invalid_large_uid" \
    "./su-exec 4294967296 echo 'test'" \
    "Test with invalid large numeric UID"

run_test "test17_negative_uid" \
    "./su-exec -1 echo 'test'" \
    "Test with negative numeric UID"

run_test "test18_user_colon_no_group" \
    "./su-exec user: echo 'test'" \
    "Test with user followed by colon but no group"

run_test "test19_only_colon" \
    "./su-exec : echo 'test'" \
    "Test with only colon (empty user and group)"

run_test "test20_multiple_colons" \
    "./su-exec user:group:extra echo 'test'" \
    "Test with multiple colons in user specification"

echo ""
echo "=== Test Summary ==="
echo "Total tests: $TOTAL_TESTS"
echo -e "Passed: ${GREEN}$PASSED_TESTS${NC}"
echo -e "Failed: ${RED}$FAILED_TESTS${NC}"

if [ $FAILED_TESTS -eq 0 ]; then
    echo -e "${GREEN}All tests passed!${NC}"
    exit 0
else
    echo -e "${RED}Some tests failed. Check the results directory for details.${NC}"
    exit 1
fi


