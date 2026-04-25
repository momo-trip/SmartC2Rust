
#!/bin/bash

# Reformed test cases

failed=0
mkdir -p flow_results

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$SCRIPT_DIR"

# Helper: prepare test_input.txt (small repetitive file)
prepare_small_input() {
    rm -f test_input.txt test_input.txt.gz test_input.txt.zlib test_input.txt.deflate
    echo "This is a test file for zopfli compression" > test_input.txt
    for i in {1..10}; do
        echo "This line is repeated multiple times to ensure compression works effectively. Line number: $i" >> test_input.txt
    done
}

# Helper: prepare larger test_input.txt
prepare_large_input() {
    rm -f test_input.txt test_input.txt.gz test_input.txt.zlib test_input.txt.deflate
    echo "This is a test file for zopfli compression" > test_input.txt
    for i in {1..100}; do
        echo "Additional line $i with some random text to compress and make the file larger" >> test_input.txt
    done
}

run_testcase() {
    local test_num="$1"
    local desc="$2"
    local binary="$3"
    local expected_output="$4"
    local format="$5"
    local check_compression="$6"
    local original_file="$7"
    shift 7
    local args=("$@")

    local log_file="flow_results/test${test_num}_tmp.log"
    : > "$log_file"

    echo "Test ${test_num} started"
    echo "Test ${test_num} started - ${desc}" >> "$log_file"

    # Remove expected output to avoid conflicts
    if [ -n "$expected_output" ]; then
        rm -f "$expected_output"
    fi

    local tc_failed=0

    LD_PRELOAD=libtracer.so TRACE_OUTPUT="$PWD/flow_results/test${test_num}_trace.log" "./$binary" "${args[@]}" >> "$log_file" 2>&1
    local rc=$?
    if [ $rc -ne 0 ]; then
        echo "Command failed with exit code $rc" >> "$log_file"
        tc_failed=1
    fi

    if [ $tc_failed -eq 0 ] && [ -n "$expected_output" ]; then
        if [ ! -f "$expected_output" ]; then
            echo "Expected output file $expected_output not found" >> "$log_file"
            tc_failed=1
        elif [ ! -s "$expected_output" ]; then
            echo "Expected output file $expected_output is empty" >> "$log_file"
            tc_failed=1
        fi
    fi

    # Integrity check
    if [ $tc_failed -eq 0 ] && [ -n "$format" ] && [ -f "$expected_output" ]; then
        case "$format" in
            gzip)
                if ! gunzip -t "$expected_output" >> "$log_file" 2>&1; then
                    echo "Gzip integrity check failed" >> "$log_file"
                    tc_failed=1
                fi
                ;;
            zlib)
                if ! python3 -c "import zlib; zlib.decompress(open('$expected_output', 'rb').read())" >> "$log_file" 2>&1; then
                    echo "Zlib integrity check failed" >> "$log_file"
                    tc_failed=1
                fi
                ;;
        esac
    fi

    # Compression check (optional)
    if [ $tc_failed -eq 0 ] && [ "$check_compression" = "yes" ] && [ -f "$original_file" ] && [ -f "$expected_output" ]; then
        local orig_size comp_size
        orig_size=$(stat -c%s "$original_file")
        comp_size=$(stat -c%s "$expected_output")
        echo "Original: ${orig_size}B, Compressed: ${comp_size}B" >> "$log_file"
        if [ "$comp_size" -ge "$orig_size" ] && [ "$orig_size" -ge 100 ]; then
            echo "Compression check failed: compressed size >= original for non-small file" >> "$log_file"
            tc_failed=1
        fi
    fi

    echo "Test ${test_num} ended"
    if [ $tc_failed -eq 0 ]; then
        echo "Test ${test_num} passed"
        mv "$log_file" "flow_results/test${test_num}_success.log"
    else
        echo "Test ${test_num} failed" >&2
        echo "Test ${test_num} failed"
        mv "$log_file" "flow_results/test${test_num}_fail.log"
        failed=1
    fi
}

# === Tests ===

# Test 1: Basic gzip compression (with compression + integrity check)
prepare_small_input
run_testcase 1 "Basic gzip compression" "build/bins/zopfli_t1" "test_input.txt.gz" "gzip" "yes" "test_input.txt" test_input.txt

# Test 2: Limited iterations (--i1)
prepare_small_input
run_testcase 2 "Limited iterations (i1)" "build/bins/zopfli_t2" "test_input.txt.gz" "gzip" "no" "" --i1 test_input.txt

# Test 3: Block splitting enabled
prepare_small_input
run_testcase 3 "Block splitting enabled" "build/bins/zopfli_t3" "test_input.txt.gz" "gzip" "no" "" --blocksplitting=1 test_input.txt

# Test 4: Zlib format
prepare_small_input
run_testcase 4 "Zlib format" "build/bins/zopfli_t4" "test_input.txt.zlib" "zlib" "yes" "test_input.txt" --zlib test_input.txt

# Test 5: Large file with verbose output
prepare_large_input
run_testcase 5 "Large file with verbose output" "build/bins/zopfli_t5" "test_input.txt.gz" "gzip" "no" "" --verbose test_input.txt

# Test 6: High iterations (i100)
prepare_large_input
run_testcase 6 "High iterations (i100)" "build/bins/zopfli_t6" "test_input.txt.gz" "gzip" "no" "" --i100 test_input.txt

# Test 7: Combined options (i50 + blocksplitting)
prepare_large_input
run_testcase 7 "Combined options (i50 + blocksplitting)" "build/bins/zopfli_t7" "test_input.txt.gz" "gzip" "no" "" --i50 --blocksplitting=1 test_input.txt

# Test 8: Block splitting max=10
prepare_large_input
run_testcase 8 "Block splitting max=10" "build/bins/zopfli_t8" "test_input.txt.gz" "gzip" "no" "" --blocksplittingmax=10 test_input.txt

# Test 9: Multiple format - zlib
prepare_large_input
run_testcase 9 "Multiple format - zlib" "build/bins/zopfli_t9" "test_input.txt.zlib" "zlib" "no" "" --zlib test_input.txt

# Test 10: Empty file compression
rm -f empty_file.txt empty_file.txt.gz
echo -n "" > empty_file.txt
run_testcase 10 "Empty file compression" "build/bins/zopfli_t10" "empty_file.txt.gz" "gzip" "no" "" empty_file.txt

# Test 11: Special characters file
rm -f special_chars.txt special_chars.txt.gz
echo "Special chars: !@#\$%^&*()_+{}|:<>?[]\\;'\",./" > special_chars.txt
run_testcase 11 "Special characters file" "build/bins/zopfli_t11" "special_chars.txt.gz" "gzip" "no" "" special_chars.txt

# Test 12: Block splitting disabled
prepare_large_input
run_testcase 12 "Block splitting disabled" "build/bins/zopfli_t12" "test_input.txt.gz" "gzip" "no" "" --i1 --blocksplitting=0 test_input.txt

# Test 13: Zlib with i1
prepare_large_input
run_testcase 13 "Zlib with i1" "build/bins/zopfli_t13" "test_input.txt.zlib" "zlib" "no" "" --i1 --zlib test_input.txt

# Test 14: Explicit gzip format
prepare_large_input
run_testcase 14 "Explicit gzip format" "build/bins/zopfli_t14" "test_input.txt.gz" "gzip" "no" "" --gzip test_input.txt

# Test 15: Verbose with block splitting
prepare_large_input
run_testcase 15 "Verbose with block splitting" "build/bins/zopfli_t15" "test_input.txt.gz" "gzip" "no" "" --verbose --blocksplitting=1 test_input.txt

# Test 16: Verbose with zlib
prepare_large_input
run_testcase 16 "Verbose with zlib" "build/bins/zopfli_t16" "test_input.txt.zlib" "zlib" "no" "" --verbose --zlib test_input.txt

# Test 17: Verbose with gzip
prepare_large_input
run_testcase 17 "Verbose with gzip" "build/bins/zopfli_t17" "test_input.txt.gz" "gzip" "no" "" --verbose --gzip test_input.txt

# Test 18: Verbose with i10 and blocksplitting
prepare_large_input
run_testcase 18 "Verbose with i10 and blocksplitting" "build/bins/zopfli_t18" "test_input.txt.gz" "gzip" "no" "" --verbose --i10 --blocksplitting=1 test_input.txt

# Test 19: High iterations i15 verbose
prepare_large_input
run_testcase 19 "High iterations i15" "build/bins/zopfli_t19" "test_input.txt.gz" "gzip" "no" "" --verbose --i15 test_input.txt

# Test 20: First compression
prepare_large_input
run_testcase 20 "First compression" "build/bins/zopfli_t20" "test_input.txt.gz" "gzip" "no" "" test_input.txt

# Test 21: Second compression with different options
prepare_large_input
run_testcase 21 "Second compression with different options" "build/bins/zopfli_t21" "test_input.txt.gz" "gzip" "no" "" --i1 test_input.txt

# Clean up test files
rm -f test_input.txt test_input.txt.gz test_input.txt.zlib test_input.txt.deflate
rm -f empty_file.txt empty_file.txt.gz
rm -f special_chars.txt special_chars.txt.gz

exit $failed

