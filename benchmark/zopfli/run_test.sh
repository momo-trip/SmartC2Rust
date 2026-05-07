
#!/bin/bash

# Reformed test cases

failed=0
mkdir -p flow_results

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$SCRIPT_DIR"

# Helper: run zopfli binary with LD_PRELOAD tracer
run_zopfli() {
    local tnum="$1"
    shift
    LD_PRELOAD=libtracer.so TRACE_OUTPUT="$PWD/flow_results/test${tnum}_trace.log" "./build/bins/zopfli_t${tnum}" "$@"
}

assert_file_compressed() {
    local original="$1"
    local compressed="$2"
    if [ -f "$original" ] && [ -f "$compressed" ]; then
        original_size=$(stat -c%s "$original")
        compressed_size=$(stat -c%s "$compressed")
        echo "DEBUG: Original: ${original_size}B, Compressed: ${compressed_size}B"
        if [ "$compressed_size" -lt "$original_size" ]; then
            return 0
        elif [ "$original_size" -lt 100 ]; then
            return 0
        else
            return 1
        fi
    else
        return 1
    fi
}

make_small_input() {
    rm -f test_input.txt test_input.txt.gz test_input.txt.zlib test_input.txt.deflate
    echo "This is a test file for zopfli compression" > test_input.txt
    for i in {1..10}; do
        echo "This line is repeated multiple times to ensure compression works effectively. Line number: $i" >> test_input.txt
    done
}

make_large_input() {
    rm -f test_input.txt test_input.txt.gz test_input.txt.zlib test_input.txt.deflate
    echo "This is a test file for zopfli compression" > test_input.txt
    for i in {1..100}; do
        echo "Additional line $i with some random text to compress and make the file larger" >> test_input.txt
    done
}

###############################
# Test 1: Basic gzip compression (also check compression size)
###############################
test_num=1
echo "Test ${test_num} started"
log="flow_results/test${test_num}_run.log"
: > "$log"
make_small_input >> "$log" 2>&1
rm -f test_input.txt.gz
run_zopfli ${test_num} test_input.txt >> "$log" 2>&1
rc=$?
pass=1
if [ $rc -ne 0 ]; then pass=0; fi
if [ ! -f test_input.txt.gz ] || [ ! -s test_input.txt.gz ]; then pass=0; fi
if [ $pass -eq 1 ]; then
    if ! assert_file_compressed test_input.txt test_input.txt.gz >> "$log" 2>&1; then pass=0; fi
    if ! gunzip -t test_input.txt.gz >> "$log" 2>&1; then pass=0; fi
fi
if [ $pass -eq 1 ]; then
    mv "$log" "flow_results/test${test_num}_success.log"
    echo "Test ${test_num} passed"
else
    mv "$log" "flow_results/test${test_num}_fail.log"
    echo "Test ${test_num} failed" >&2
    failed=1
fi
echo "Test ${test_num} ended"

###############################
# Test 2: Limited iterations (i1)
###############################
test_num=2
echo "Test ${test_num} started"
log="flow_results/test${test_num}_run.log"
: > "$log"
make_small_input >> "$log" 2>&1
rm -f test_input.txt.gz
run_zopfli ${test_num} --i1 test_input.txt >> "$log" 2>&1
rc=$?
if [ $rc -eq 0 ] && [ -s test_input.txt.gz ]; then
    mv "$log" "flow_results/test${test_num}_success.log"
    echo "Test ${test_num} passed"
else
    mv "$log" "flow_results/test${test_num}_fail.log"
    echo "Test ${test_num} failed" >&2
    failed=1
fi
echo "Test ${test_num} ended"

###############################
# Test 3: Block splitting enabled
###############################
test_num=3
echo "Test ${test_num} started"
log="flow_results/test${test_num}_run.log"
: > "$log"
make_small_input >> "$log" 2>&1
rm -f test_input.txt.gz
run_zopfli ${test_num} --blocksplitting=1 test_input.txt >> "$log" 2>&1
rc=$?
if [ $rc -eq 0 ] && [ -s test_input.txt.gz ]; then
    mv "$log" "flow_results/test${test_num}_success.log"
    echo "Test ${test_num} passed"
else
    mv "$log" "flow_results/test${test_num}_fail.log"
    echo "Test ${test_num} failed" >&2
    failed=1
fi
echo "Test ${test_num} ended"

###############################
# Test 4: Zlib format (includes compression-size and integrity sub-checks)
###############################
test_num=4
echo "Test ${test_num} started"
log="flow_results/test${test_num}_run.log"
: > "$log"
make_small_input >> "$log" 2>&1
rm -f test_input.txt.zlib
run_zopfli ${test_num} --zlib test_input.txt >> "$log" 2>&1
rc=$?
pass=1
if [ $rc -ne 0 ]; then pass=0; fi
if [ ! -s test_input.txt.zlib ]; then pass=0; fi
if [ $pass -eq 1 ]; then
    if ! assert_file_compressed test_input.txt test_input.txt.zlib >> "$log" 2>&1; then pass=0; fi
    if ! python3 -c "import zlib; zlib.decompress(open('test_input.txt.zlib','rb').read())" >> "$log" 2>&1; then pass=0; fi
fi
if [ $pass -eq 1 ]; then
    mv "$log" "flow_results/test${test_num}_success.log"
    echo "Test ${test_num} passed"
else
    mv "$log" "flow_results/test${test_num}_fail.log"
    echo "Test ${test_num} failed" >&2
    failed=1
fi
echo "Test ${test_num} ended"

###############################
# Test 5: Large file with verbose output
###############################
test_num=5
echo "Test ${test_num} started"
log="flow_results/test${test_num}_run.log"
: > "$log"
make_large_input >> "$log" 2>&1
rm -f test_input.txt.gz
run_zopfli ${test_num} --verbose test_input.txt >> "$log" 2>&1
rc=$?
if [ $rc -eq 0 ] && [ -s test_input.txt.gz ]; then
    mv "$log" "flow_results/test${test_num}_success.log"
    echo "Test ${test_num} passed"
else
    mv "$log" "flow_results/test${test_num}_fail.log"
    echo "Test ${test_num} failed" >&2
    failed=1
fi
echo "Test ${test_num} ended"

###############################
# Test 6: High iterations (i100)
###############################
test_num=6
echo "Test ${test_num} started"
log="flow_results/test${test_num}_run.log"
: > "$log"
make_large_input >> "$log" 2>&1
rm -f test_input.txt.gz
run_zopfli ${test_num} --i100 test_input.txt >> "$log" 2>&1
rc=$?
if [ $rc -eq 0 ] && [ -s test_input.txt.gz ]; then
    mv "$log" "flow_results/test${test_num}_success.log"
    echo "Test ${test_num} passed"
else
    mv "$log" "flow_results/test${test_num}_fail.log"
    echo "Test ${test_num} failed" >&2
    failed=1
fi
echo "Test ${test_num} ended"

###############################
# Test 7: Combined options (i50 + blocksplitting)
###############################
test_num=7
echo "Test ${test_num} started"
log="flow_results/test${test_num}_run.log"
: > "$log"
make_large_input >> "$log" 2>&1
rm -f test_input.txt.gz
run_zopfli ${test_num} --i50 --blocksplitting=1 test_input.txt >> "$log" 2>&1
rc=$?
if [ $rc -eq 0 ] && [ -s test_input.txt.gz ]; then
    mv "$log" "flow_results/test${test_num}_success.log"
    echo "Test ${test_num} passed"
else
    mv "$log" "flow_results/test${test_num}_fail.log"
    echo "Test ${test_num} failed" >&2
    failed=1
fi
echo "Test ${test_num} ended"

###############################
# Test 8: Block splitting max=10
###############################
test_num=8
echo "Test ${test_num} started"
log="flow_results/test${test_num}_run.log"
: > "$log"
make_large_input >> "$log" 2>&1
rm -f test_input.txt.gz
run_zopfli ${test_num} --blocksplittingmax=10 test_input.txt >> "$log" 2>&1
rc=$?
if [ $rc -eq 0 ] && [ -s test_input.txt.gz ]; then
    mv "$log" "flow_results/test${test_num}_success.log"
    echo "Test ${test_num} passed"
else
    mv "$log" "flow_results/test${test_num}_fail.log"
    echo "Test ${test_num} failed" >&2
    failed=1
fi
echo "Test ${test_num} ended"

###############################
# Test 9: Multiple format - zlib (on large file)
###############################
test_num=9
echo "Test ${test_num} started"
log="flow_results/test${test_num}_run.log"
: > "$log"
make_large_input >> "$log" 2>&1
rm -f test_input.txt.zlib
run_zopfli ${test_num} --zlib test_input.txt >> "$log" 2>&1
rc=$?
if [ $rc -eq 0 ] && [ -s test_input.txt.zlib ]; then
    mv "$log" "flow_results/test${test_num}_success.log"
    echo "Test ${test_num} passed"
else
    mv "$log" "flow_results/test${test_num}_fail.log"
    echo "Test ${test_num} failed" >&2
    failed=1
fi
echo "Test ${test_num} ended"

###############################
# Test 10: Empty file compression (with integrity check)
###############################
test_num=10
echo "Test ${test_num} started"
log="flow_results/test${test_num}_run.log"
: > "$log"
rm -f empty_file.txt empty_file.txt.gz
echo -n "" > empty_file.txt
run_zopfli ${test_num} empty_file.txt >> "$log" 2>&1
rc=$?
pass=1
if [ $rc -ne 0 ]; then pass=0; fi
if [ ! -f empty_file.txt.gz ] || [ ! -s empty_file.txt.gz ]; then pass=0; fi
if [ $pass -eq 1 ]; then
    if ! gunzip -t empty_file.txt.gz >> "$log" 2>&1; then pass=0; fi
fi
rm -f empty_file.txt empty_file.txt.gz
if [ $pass -eq 1 ]; then
    mv "$log" "flow_results/test${test_num}_success.log"
    echo "Test ${test_num} passed"
else
    mv "$log" "flow_results/test${test_num}_fail.log"
    echo "Test ${test_num} failed" >&2
    failed=1
fi
echo "Test ${test_num} ended"

###############################
# Test 11: Special characters file
###############################
test_num=11
echo "Test ${test_num} started"
log="flow_results/test${test_num}_run.log"
: > "$log"
rm -f special_chars.txt special_chars.txt.gz
echo "Special chars: !@#\$%^&*()_+{}|:<>?[]\\;'\",./" > special_chars.txt
run_zopfli ${test_num} special_chars.txt >> "$log" 2>&1
rc=$?
if [ $rc -eq 0 ] && [ -s special_chars.txt.gz ]; then
    rm -f special_chars.txt special_chars.txt.gz
    mv "$log" "flow_results/test${test_num}_success.log"
    echo "Test ${test_num} passed"
else
    rm -f special_chars.txt special_chars.txt.gz
    mv "$log" "flow_results/test${test_num}_fail.log"
    echo "Test ${test_num} failed" >&2
    failed=1
fi
echo "Test ${test_num} ended"

###############################
# Test 12: Block splitting disabled
###############################
test_num=12
echo "Test ${test_num} started"
log="flow_results/test${test_num}_run.log"
: > "$log"
make_large_input >> "$log" 2>&1
rm -f test_input.txt.gz
run_zopfli ${test_num} --i1 --blocksplitting=0 test_input.txt >> "$log" 2>&1
rc=$?
if [ $rc -eq 0 ] && [ -s test_input.txt.gz ]; then
    mv "$log" "flow_results/test${test_num}_success.log"
    echo "Test ${test_num} passed"
else
    mv "$log" "flow_results/test${test_num}_fail.log"
    echo "Test ${test_num} failed" >&2
    failed=1
fi
echo "Test ${test_num} ended"

###############################
# Test 13: Zlib with i1
###############################
test_num=13
echo "Test ${test_num} started"
log="flow_results/test${test_num}_run.log"
: > "$log"
make_large_input >> "$log" 2>&1
rm -f test_input.txt.zlib
run_zopfli ${test_num} --i1 --zlib test_input.txt >> "$log" 2>&1
rc=$?
if [ $rc -eq 0 ] && [ -s test_input.txt.zlib ]; then
    mv "$log" "flow_results/test${test_num}_success.log"
    echo "Test ${test_num} passed"
else
    mv "$log" "flow_results/test${test_num}_fail.log"
    echo "Test ${test_num} failed" >&2
    failed=1
fi
echo "Test ${test_num} ended"

###############################
# Test 14: Explicit gzip format
###############################
test_num=14
echo "Test ${test_num} started"
log="flow_results/test${test_num}_run.log"
: > "$log"
make_large_input >> "$log" 2>&1
rm -f test_input.txt.gz
run_zopfli ${test_num} --gzip test_input.txt >> "$log" 2>&1
rc=$?
if [ $rc -eq 0 ] && [ -s test_input.txt.gz ]; then
    mv "$log" "flow_results/test${test_num}_success.log"
    echo "Test ${test_num} passed"
else
    mv "$log" "flow_results/test${test_num}_fail.log"
    echo "Test ${test_num} failed" >&2
    failed=1
fi
echo "Test ${test_num} ended"

###############################
# Test 15: Verbose with block splitting
###############################
test_num=15
echo "Test ${test_num} started"
log="flow_results/test${test_num}_run.log"
: > "$log"
make_large_input >> "$log" 2>&1
rm -f test_input.txt.gz
run_zopfli ${test_num} --verbose --blocksplitting=1 test_input.txt >> "$log" 2>&1
rc=$?
if [ $rc -eq 0 ] && [ -s test_input.txt.gz ]; then
    mv "$log" "flow_results/test${test_num}_success.log"
    echo "Test ${test_num} passed"
else
    mv "$log" "flow_results/test${test_num}_fail.log"
    echo "Test ${test_num} failed" >&2
    failed=1
fi
echo "Test ${test_num} ended"

###############################
# Test 16: Verbose with zlib
###############################
test_num=16
echo "Test ${test_num} started"
log="flow_results/test${test_num}_run.log"
: > "$log"
make_large_input >> "$log" 2>&1
rm -f test_input.txt.zlib
run_zopfli ${test_num} --verbose --zlib test_input.txt >> "$log" 2>&1
rc=$?
if [ $rc -eq 0 ] && [ -s test_input.txt.zlib ]; then
    mv "$log" "flow_results/test${test_num}_success.log"
    echo "Test ${test_num} passed"
else
    mv "$log" "flow_results/test${test_num}_fail.log"
    echo "Test ${test_num} failed" >&2
    failed=1
fi
echo "Test ${test_num} ended"

###############################
# Test 17: Verbose with gzip
###############################
test_num=17
echo "Test ${test_num} started"
log="flow_results/test${test_num}_run.log"
: > "$log"
make_large_input >> "$log" 2>&1
rm -f test_input.txt.gz
run_zopfli ${test_num} --verbose --gzip test_input.txt >> "$log" 2>&1
rc=$?
if [ $rc -eq 0 ] && [ -s test_input.txt.gz ]; then
    mv "$log" "flow_results/test${test_num}_success.log"
    echo "Test ${test_num} passed"
else
    mv "$log" "flow_results/test${test_num}_fail.log"
    echo "Test ${test_num} failed" >&2
    failed=1
fi
echo "Test ${test_num} ended"

###############################
# Test 18: Verbose with i10 and blocksplitting
###############################
test_num=18
echo "Test ${test_num} started"
log="flow_results/test${test_num}_run.log"
: > "$log"
make_large_input >> "$log" 2>&1
rm -f test_input.txt.gz
run_zopfli ${test_num} --verbose --i10 --blocksplitting=1 test_input.txt >> "$log" 2>&1
rc=$?
if [ $rc -eq 0 ] && [ -s test_input.txt.gz ]; then
    mv "$log" "flow_results/test${test_num}_success.log"
    echo "Test ${test_num} passed"
else
    mv "$log" "flow_results/test${test_num}_fail.log"
    echo "Test ${test_num} failed" >&2
    failed=1
fi
echo "Test ${test_num} ended"

###############################
# Test 19: High iterations i15
###############################
test_num=19
echo "Test ${test_num} started"
log="flow_results/test${test_num}_run.log"
: > "$log"
make_large_input >> "$log" 2>&1
rm -f test_input.txt.gz
run_zopfli ${test_num} --verbose --i15 test_input.txt >> "$log" 2>&1
rc=$?
if [ $rc -eq 0 ] && [ -s test_input.txt.gz ]; then
    mv "$log" "flow_results/test${test_num}_success.log"
    echo "Test ${test_num} passed"
else
    mv "$log" "flow_results/test${test_num}_fail.log"
    echo "Test ${test_num} failed" >&2
    failed=1
fi
echo "Test ${test_num} ended"

###############################
# Test 20: First compression
###############################
test_num=20
echo "Test ${test_num} started"
log="flow_results/test${test_num}_run.log"
: > "$log"
make_large_input >> "$log" 2>&1
rm -f test_input.txt.gz
run_zopfli ${test_num} test_input.txt >> "$log" 2>&1
rc=$?
if [ $rc -eq 0 ] && [ -s test_input.txt.gz ]; then
    mv "$log" "flow_results/test${test_num}_success.log"
    echo "Test ${test_num} passed"
else
    mv "$log" "flow_results/test${test_num}_fail.log"
    echo "Test ${test_num} failed" >&2
    failed=1
fi
echo "Test ${test_num} ended"

###############################
# Test 21: Second compression with different options (--i1)
###############################
test_num=21
echo "Test ${test_num} started"
log="flow_results/test${test_num}_run.log"
: > "$log"
make_large_input >> "$log" 2>&1
rm -f test_input.txt.gz
run_zopfli ${test_num} --i1 test_input.txt >> "$log" 2>&1
rc=$?
if [ $rc -eq 0 ] && [ -s test_input.txt.gz ]; then
    mv "$log" "flow_results/test${test_num}_success.log"
    echo "Test ${test_num} passed"
else
    mv "$log" "flow_results/test${test_num}_fail.log"
    echo "Test ${test_num} failed" >&2
    failed=1
fi
echo "Test ${test_num} ended"

# Final cleanup
rm -f test_input.txt test_input.txt.gz test_input.txt.zlib test_input.txt.deflate
rm -f empty_file.txt empty_file.txt.gz
rm -f special_chars.txt special_chars.txt.gz

exit $failed

