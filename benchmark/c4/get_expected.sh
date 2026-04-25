#!/bin/bash
# generate_c4_expected.sh - Script to save C4 compiler execution results as expected values in the expected directory
# Normalizes environment-dependent values like address values before saving

# Color settings
GREEN='\033[0;32m'
RED='\033[0;31m'
BLUE='\033[0;34m'
YELLOW='\033[0;33m'
NC='\033[0m' # No Color

# Directory settings
EXPECTED_DIR="./expected"

# C4 compiler and test file paths
C4_COMPILER="./c4"
HELLO_C="hello.c"
ARGINC_C="arginc.c"
C4_C="c4.c"

# Remove existing directory and recreate
rm -rf "$EXPECTED_DIR"
mkdir -p "$EXPECTED_DIR"

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

# Check existence of compiler and test files
if [ ! -f "$C4_COMPILER" ]; then
    print_error "C4 compiler not found: $C4_COMPILER"
fi

if [ ! -f "$HELLO_C" ]; then
    print_error "Test file not found: $HELLO_C"
fi

if [ ! -f "$ARGINC_C" ]; then
    print_error "Test file not found: $ARGINC_C"
fi

# Generate expected results
print_header "Saving C4 compiler execution results"

# Test 1: Execute hello.c
print_info "Test 1: Compile and execute hello.c"
temp_file=$(mktemp)
$C4_COMPILER $HELLO_C > "$temp_file" 2>&1
if [ $? -eq 0 ]; then
    normalize "$temp_file" "$EXPECTED_DIR/test1_result.txt"
    print_success "hello.c execution result saved: $EXPECTED_DIR/test1_result.txt"
else
    print_error "hello.c execution failed"
fi
rm -f "$temp_file"

# Test 2: Compile hello.c and display intermediate code
print_info "Test 2: Display intermediate code for hello.c"
temp_file=$(mktemp)
$C4_COMPILER -s $HELLO_C > "$temp_file" 2>&1
if [ $? -eq 0 ]; then
    normalize "$temp_file" "$EXPECTED_DIR/test2_result.txt"
    print_success "hello.c intermediate code display result saved: $EXPECTED_DIR/test2_result.txt"
else
    print_error "hello.c intermediate code display failed"
fi
rm -f "$temp_file"

# Test 3: Compile and execute arginc.c
print_info "Test 3: Compile and execute arginc.c"
temp_file=$(mktemp)
$C4_COMPILER $ARGINC_C > "$temp_file" 2>&1
actual_exit=$?
normalize "$temp_file" "$EXPECTED_DIR/test3_result.txt"
echo "$actual_exit" > "$EXPECTED_DIR/test3_exit_code.txt"
print_success "Execution result saved: $EXPECTED_DIR/test3_result.txt (exit=$actual_exit)"
rm -f "$temp_file"


# Test 4: Display intermediate code for arginc.c
print_info "Test 4: Display intermediate code for arginc.c"
temp_file=$(mktemp)
$C4_COMPILER -s $ARGINC_C > "$temp_file" 2>&1
if [ $? -eq 0 ]; then
    normalize "$temp_file" "$EXPECTED_DIR/test4_result.txt"
    print_success "Execution result saved: $EXPECTED_DIR/test4_result.txt"
else
    print_error "Execution failed"
fi
rm -f "$temp_file"