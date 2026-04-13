#!/bin/bash

# run_c4_test.sh - Script to run C4 compiler tests and compare against expected values
# Normalizes environment-dependent values like address values for comparison

# Color settings
GREEN='\033[0;32m'
RED='\033[0;31m'
BLUE='\033[0;34m'
YELLOW='\033[0;33m'
NC='\033[0m' # No Color

# Directory settings
EXPECTED_DIR="./expected"
RESULTS_DIR="./test-results"
TEMP_DIR="./test-temp"

# C4 compiler and test file paths
C4_COMPILER="./c4"
HELLO_C="hello.c"
ARGINC_C="arginc.c"
C4_C="c4.c"

# Remove and recreate test result directories
rm -rf "$RESULTS_DIR"
rm -rf "$TEMP_DIR"
mkdir -p "$RESULTS_DIR"
mkdir -p "$TEMP_DIR"

# Log files
C_LOG_FILE="/home/ubuntu/portable/out_flow_c.log"
RUST_LOG_FILE="/home/ubuntu/portable/out_flow_rust.log"

# Log start of a test
logstart() {
    local test_num=$1
    local test_name=$2
    
    echo "Test Case #${test_num}: Started" >> $C_LOG_FILE
    echo "Test Case #${test_num}: Started" >> $RUST_LOG_FILE
}

# Helper functions
print_header() {
    echo -e "\n${BLUE}==== $1 ====${NC}\n"
}

print_info() {
    echo -e "${YELLOW}[INFO]${NC} $1"
}

print_success() {
    echo -e "${GREEN}[SUCCESS]${NC} $1"
}

print_error() {
    echo -e "${RED}[FAILED]${NC} $1"
}

print_fatal() {
    echo -e "${RED}[ERROR]${NC} $1"
    exit 1
}

# Normalization function - Normalize environment-dependent values like numbers after IMM instructions and cycle values
normalize() {
    local input_file="$1"
    local output_file="$2"
    
    # Normalize numbers after IMM instructions
    # Normalize cycle values
    # Normalize LEA instructions
    # Normalize exit codes
    cat "$input_file" | \
        sed 's/IMM  [-0-9][0-9]*/IMM  XXXX/g' | \
        sed 's/cycle = [0-9][0-9]*/cycle = XXXX/g' | \
        sed 's/LEA  [0-9][0-9]*/LEA  XXXX/g' | \
        sed 's/exit([0-9][0-9]*)/exit(XXXX)/g' > "$output_file"
    
    print_info "File normalized: $output_file"
}

# Check for required files and directories
if [ ! -d "$EXPECTED_DIR" ]; then
    print_fatal "Expected results directory not found: $EXPECTED_DIR"
    echo "Please run generate_c4_expected.sh first to generate expected results."
fi

if [ ! -f "$C4_COMPILER" ]; then
    print_fatal "C4 compiler not found: $C4_COMPILER"
fi

if [ ! -f "$HELLO_C" ]; then
    print_fatal "Test file not found: $HELLO_C"
fi

if [ ! -f "$C4_C" ]; then
    print_fatal "C4 source file not found: $C4_C"
fi

for file in test1_result.txt test2_result.txt test3_result.txt test4_result.txt; do
    if [ ! -f "$EXPECTED_DIR/$file" ]; then
        print_fatal "Expected result file not found: $EXPECTED_DIR/$file"
        echo "Please run generate_c4_expected.sh first to generate expected results."
    fi
done

# Test execution
print_header "C4 compiler test execution"

# Test 1: Execute hello.c
logstart "1" "Compile and execute hello.c"
print_info "Test 1: Compile and execute hello.c"
echo -e "${BLUE}--- Test 1 Output ---${NC}"
temp_file="$TEMP_DIR/test1_raw.txt"
$C4_COMPILER $HELLO_C 2>&1 | tee "$temp_file"
echo -e "${BLUE}--- End Test 1 Output ---${NC}"
if [ ${PIPESTATUS[0]} -eq 0 ]; then
    normalize "$temp_file" "$RESULTS_DIR/test1_result.txt"
    print_success "hello.c execution succeeded"
else
    print_error "hello.c execution failed"
fi

# Test 2: Compile hello.c and display intermediate code
logstart "2" "Display intermediate code for hello.c"
print_info "Test 2: Display intermediate code for hello.c"
echo -e "${BLUE}--- Test 2 Output ---${NC}"
temp_file="$TEMP_DIR/test2_raw.txt"
$C4_COMPILER -s $HELLO_C 2>&1 | tee "$temp_file"
echo -e "${BLUE}--- End Test 2 Output ---${NC}"
if [ ${PIPESTATUS[0]} -eq 0 ]; then
    normalize "$temp_file" "$RESULTS_DIR/test2_result.txt"
    print_success "hello.c intermediate code display succeeded"
else
    print_error "hello.c intermediate code display failed"
fi

# Test 3: Compile and execute arginc.c
logstart "3" "Compile and execute arginc.c"
print_info "Test 3: Compile and execute arginc.c"
echo -e "${BLUE}--- Test 3 Output ---${NC}"
temp_file="$TEMP_DIR/test3_raw.txt"
$C4_COMPILER $ARGINC_C 2>&1 | tee "$temp_file"
echo -e "${BLUE}--- End Test 3 Output ---${NC}"
if [ ${PIPESTATUS[0]} -eq 0 ]; then
    normalize "$temp_file" "$RESULTS_DIR/test3_result.txt"
    print_success "arginc.c execution succeeded"
else
    print_error "arginc.c execution failed"
fi

# Test 4: Display intermediate code for arginc.c
logstart "4" "Display intermediate code for arginc.c"
print_info "Test 4: Display intermediate code for arginc.c"
echo -e "${BLUE}--- Test 4 Output ---${NC}"
temp_file="$TEMP_DIR/test4_raw.txt"
$C4_COMPILER -s $ARGINC_C 2>&1 | tee "$temp_file"
echo -e "${BLUE}--- End Test 4 Output ---${NC}"
if [ ${PIPESTATUS[0]} -eq 0 ]; then
    normalize "$temp_file" "$RESULTS_DIR/test4_result.txt"
    print_success "arginc.c intermediate code display succeeded"
else
    print_error "arginc.c intermediate code display failed"
fi

# Compare test result files
compare_results() {
    local test_num="$1"
    local description="$2"
    local expected_file="$EXPECTED_DIR/test${test_num}_result.txt"
    local actual_file="$RESULTS_DIR/test${test_num}_result.txt"
    
    # Compare file contents
    if diff -q "$actual_file" "$expected_file" >/dev/null; then
        print_success "Test $test_num: SUCCESS - Result matches expected value"
        return 0
    else
        print_error "Test $test_num: FAILED - Result differs from expected value"
        echo -e "${YELLOW}Expected result:${NC}"
        cat "$expected_file"
        echo -e "${YELLOW}Actual result:${NC}"
        cat "$actual_file"
        echo -e "${YELLOW}Difference:${NC}"
        diff "$expected_file" "$actual_file"
        return 1
    fi
}

# Test result report
REPORT_FILE="$RESULTS_DIR/test_report.txt"
FAILURES=0
TOTAL=0

echo "# C4 Compiler Test Results Report" > $REPORT_FILE
echo "Generated: $(date)" >> $REPORT_FILE
echo "" >> $REPORT_FILE
echo "Note: This test normalizes environment-dependent values (memory addresses, cycle counts, etc.)" >> $REPORT_FILE
echo "" >> $REPORT_FILE

# Compare each test result
print_header "Test result comparison"

# Test definitions
declare -A test_descriptions
test_descriptions[1]="Compile and execute hello.c"
test_descriptions[2]="Display intermediate code for hello.c"
test_descriptions[3]="Compile c4.c and execute arginc.c"
test_descriptions[4]="Display intermediate code for arginc.c"

# Compare each test result
for test_num in 1 2 3 4; do
    ((TOTAL++))
    
    echo "## Test $test_num: ${test_descriptions[$test_num]}" >> $REPORT_FILE
    
    if compare_results "$test_num" "${test_descriptions[$test_num]}"; then
        echo "Result: SUCCESS ✅" >> $REPORT_FILE
    else
        echo "Result: FAILED ❌" >> $REPORT_FILE
        ((FAILURES++))
    fi
    
    echo "" >> $REPORT_FILE
done

# Add test result summary
echo "# Test Result Summary" >> $REPORT_FILE
echo "* Total tests executed: $TOTAL" >> $REPORT_FILE
echo "* Passed: $((TOTAL-FAILURES))" >> $REPORT_FILE
echo "* Failed: $FAILURES" >> $REPORT_FILE

# Display final results
print_header "Test result summary"
echo "Total tests executed: $TOTAL"
echo "Passed: $((TOTAL-FAILURES))"
echo "Failed: $FAILURES"

print_info "Detailed test report created: $REPORT_FILE"

# Display the test report to stdout as well
print_header "Test report"
cat "$REPORT_FILE"

if [ $FAILURES -eq 0 ]; then
    print_success "All tests passed!"
    exit 0
else
    print_error "$FAILURES test(s) failed. Please check the report for details."
    exit 1
fi