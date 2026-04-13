#!/bin/bash

# Test script for zopfli with assertions
#set -e  # Exit on any error

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# Test counter
TEST_COUNT=0
PASS_COUNT=0
FAIL_COUNT=0

# Function to run a test with assertion
run_test() {
    local test_name="$1"
    local command="$2"
    local expected_output_file="$3"
    
    TEST_COUNT=$((TEST_COUNT + 1))
    echo -e "${YELLOW}[TEST $TEST_COUNT] $test_name${NC}"
    
    # Clean up any existing output file to avoid conflicts
    if [ -n "$expected_output_file" ]; then
        rm -f "$expected_output_file"
    fi
    
    # Run the command and capture exit code
    if eval "$command"; then
        # Check if expected output file exists
        if [ -n "$expected_output_file" ] && [ -f "$expected_output_file" ]; then
            # Additional checks
            if [ -s "$expected_output_file" ]; then
                echo -e "${GREEN}✓ PASS: $test_name${NC}"
                PASS_COUNT=$((PASS_COUNT + 1))
                return 0
            else
                echo -e "${RED}✗ FAIL: $test_name - Output file is empty${NC}"
                FAIL_COUNT=$((FAIL_COUNT + 1))
                return 1
            fi
        elif [ -z "$expected_output_file" ]; then
            echo -e "${GREEN}✓ PASS: $test_name${NC}"
            PASS_COUNT=$((PASS_COUNT + 1))
            return 0
        else
            echo -e "${RED}✗ FAIL: $test_name - Expected output file not found${NC}"
            FAIL_COUNT=$((FAIL_COUNT + 1))
            return 1
        fi
    else
        echo -e "${RED}✗ FAIL: $test_name - Command failed${NC}"
        FAIL_COUNT=$((FAIL_COUNT + 1))
        return 1
    fi
}

# Function to compare file sizes
assert_file_compressed() {
    local original="$1"
    local compressed="$2"
    local test_name="$3"
    
    if [ -f "$original" ] && [ -f "$compressed" ]; then
        original_size=$(stat -c%s "$original")
        compressed_size=$(stat -c%s "$compressed")
        
        echo -e "${YELLOW}DEBUG: $test_name - Original: ${original_size}B, Compressed: ${compressed_size}B${NC}"
        
        # For very small files, compression might actually increase size due to headers
        # So we check if compressed file exists and is valid instead of just size
        if [ "$compressed_size" -lt "$original_size" ]; then
            echo -e "${GREEN}✓ COMPRESSION CHECK: $test_name - File successfully compressed${NC}"
            return 0
        elif [ "$original_size" -lt 100 ]; then
            # Small files might not compress well due to header overhead
            echo -e "${YELLOW}✓ COMPRESSION CHECK: $test_name - Small file, compression overhead expected${NC}"
            return 0
        else
            echo -e "${RED}✗ COMPRESSION CHECK: $test_name - Large file not compressed properly${NC}"
            return 1
        fi
    else
        echo -e "${RED}✗ COMPRESSION CHECK: $test_name - Files not found (original: $([ -f "$original" ] && echo "exists" || echo "missing"), compressed: $([ -f "$compressed" ] && echo "exists" || echo "missing"))${NC}"
        return 1
    fi
}

# Function to verify compressed file integrity (now counts as separate test)
verify_compressed_file() {
    local compressed_file="$1"
    local format="$2"
    local test_name="$3"
    
    TEST_COUNT=$((TEST_COUNT + 1))
    echo -e "${YELLOW}[TEST $TEST_COUNT] $test_name - Integrity Check${NC}"
    
    case "$format" in
        "gzip")
            if gunzip -t "$compressed_file" 2>/dev/null; then
                echo -e "${GREEN}✓ PASS: $test_name - Gzip file is valid${NC}"
                PASS_COUNT=$((PASS_COUNT + 1))
                return 0
            else
                echo -e "${RED}✗ FAIL: $test_name - Gzip file is corrupted${NC}"
                FAIL_COUNT=$((FAIL_COUNT + 1))
                return 1
            fi
            ;;
        "zlib")
            # For zlib, we can try to decompress with python
            if python3 -c "import zlib; zlib.decompress(open('$compressed_file', 'rb').read())" 2>/dev/null; then
                echo -e "${GREEN}✓ PASS: $test_name - Zlib file is valid${NC}"
                PASS_COUNT=$((PASS_COUNT + 1))
                return 0
            else
                echo -e "${RED}✗ FAIL: $test_name - Zlib file is corrupted${NC}"
                FAIL_COUNT=$((FAIL_COUNT + 1))
                return 1
            fi
            ;;
        *)
            echo -e "${YELLOW}? SKIP: $test_name - Unknown format, skipping integrity check${NC}"
            # Don't count skipped tests in pass/fail counts
            TEST_COUNT=$((TEST_COUNT - 1))
            return 0
            ;;
    esac
}

# Function to run compression size check as separate test
run_compression_check() {
    local original="$1"
    local compressed="$2"
    local test_name="$3"
    
    TEST_COUNT=$((TEST_COUNT + 1))
    echo -e "${YELLOW}[TEST $TEST_COUNT] $test_name - Compression Check${NC}"
    
    if assert_file_compressed "$original" "$compressed" "$test_name"; then
        PASS_COUNT=$((PASS_COUNT + 1))
        return 0
    else
        FAIL_COUNT=$((FAIL_COUNT + 1))
        return 1
    fi
}

echo "=== Zopfli Test Suite ==="
echo

# Clean up any existing files
rm -f test_input.txt test_input.txt.gz test_input.txt.zlib test_input.txt.deflate

# Create initial test file
echo "This is a test file for zopfli compression" > test_input.txt
# Add some repetitive content to ensure compression works
for i in {1..10}; do
    echo "This line is repeated multiple times to ensure compression works effectively. Line number: $i" >> test_input.txt
done

# Test 1: Basic compression
run_test "Basic gzip compression" "./build/zopfli test_input.txt" "test_input.txt.gz"
if [ -f "test_input.txt.gz" ]; then
    run_compression_check "test_input.txt" "test_input.txt.gz" "Basic gzip compression"
    verify_compressed_file "test_input.txt.gz" "gzip" "Basic gzip compression"
fi

# Test 2: Limited iterations
run_test "Limited iterations (i1)" "./build/zopfli --i1 test_input.txt" "test_input.txt.gz"

# Test 3: Block splitting
run_test "Block splitting enabled" "./build/zopfli --blocksplitting=1 test_input.txt" "test_input.txt.gz"

# Test 4: Zlib format
run_test "Zlib format" "./build/zopfli --zlib test_input.txt" "test_input.txt.zlib"
if [ -f "test_input.txt.zlib" ]; then
    run_compression_check "test_input.txt" "test_input.txt.zlib" "Zlib format"
    verify_compressed_file "test_input.txt.zlib" "zlib" "Zlib format"
fi

# Clean up and create larger test file
rm -f test_input.txt test_input.txt.gz test_input.txt.zlib test_input.txt.deflate

# Create a larger test file
echo "This is a test file for zopfli compression" > test_input.txt
for i in {1..100}; do
    echo "Additional line $i with some random text to compress and make the file larger" >> test_input.txt
done

# Test 5: Large file with verbose output
run_test "Large file with verbose output" "./build/zopfli --verbose test_input.txt" "test_input.txt.gz"

# Test 6: High iterations
run_test "High iterations (i100)" "./build/zopfli --i100 test_input.txt" "test_input.txt.gz"

# Test 7: Combined options
run_test "Combined options (i50 + blocksplitting)" "./build/zopfli --i50 --blocksplitting=1 test_input.txt" "test_input.txt.gz"

# Test 8: Block splitting max
run_test "Block splitting max=10" "./build/zopfli --blocksplittingmax=10 test_input.txt" "test_input.txt.gz"

# Test 9: Multiple format tests
run_test "Multiple format - zlib" "./build/zopfli --zlib test_input.txt" "test_input.txt.zlib"

# Test 10: Edge case - empty file
echo -n "" > empty_file.txt

# Test 11: Test with special characters
echo "Special chars: !@#$%^&*()_+{}|:<>?[]\\;'\",./" > special_chars.txt
run_test "Special characters file" "./build/zopfli special_chars.txt" "special_chars.txt.gz"

# Test 12: Block splitting disabled
run_test "Block splitting disabled" "./build/zopfli --i1 --blocksplitting=0 test_input.txt" "test_input.txt.gz"

# # Test 13: Deflate with limited iterations
# run_test "Deflate with i1" "./build/zopfli --i1 --deflate test_input.txt" "test_input.txt.deflate"

# Test 13: Zlib with limited iterations
run_test "Zlib with i1" "./build/zopfli --i1 --zlib test_input.txt" "test_input.txt.zlib"

# Test 14: Gzip format explicitly with iterations
run_test "Explicit gzip format" "./build/zopfli --gzip test_input.txt" "test_input.txt.gz"

# Test 15: Verbose with block splitting
run_test "Verbose with block splitting" "./build/zopfli --verbose --blocksplitting=1 test_input.txt" "test_input.txt.gz"

# Test 16: Verbose with zlib format
run_test "Verbose with zlib" "./build/zopfli --verbose --zlib test_input.txt" "test_input.txt.zlib"

# Test 17: Verbose with gzip format
run_test "Verbose with gzip" "./build/zopfli --verbose --gzip test_input.txt" "test_input.txt.gz"

# Test 18: Combined verbose options
run_test "Verbose with i10 and blocksplitting" "./build/zopfli --verbose --i10 --blocksplitting=1 test_input.txt" "test_input.txt.gz"

# Test 19: High iterations (i15)
run_test "High iterations i15" "./build/zopfli --verbose --i15 test_input.txt" "test_input.txt.gz"

# Test 20: Multiple compressions on same file
run_test "First compression" "./build/zopfli test_input.txt" "test_input.txt.gz"
rm -f test_input.txt.gz
run_test "Second compression with different options" "./build/zopfli --i1 test_input.txt" "test_input.txt.gz"

# Clean up test files
rm -f test_input.txt test_input.txt.gz test_input.txt.zlib test_input.txt.deflate
rm -f empty_file.txt empty_file.txt.gz
rm -f special_chars.txt special_chars.txt.gz

echo
echo "=== Test Results ==="
echo -e "Total tests: $TEST_COUNT"
echo -e "${GREEN}Passed: $PASS_COUNT${NC}"
echo -e "${RED}Failed: $FAIL_COUNT${NC}"

if [ $FAIL_COUNT -eq 0 ]; then
    echo -e "${GREEN}All tests passed!${NC}"
    exit 0
else
    echo -e "${RED}Some tests failed!${NC}"
    exit 1
fi