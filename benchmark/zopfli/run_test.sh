
#!/bin/bash

# Reformed test cases

failed=0
mkdir -p flow_results

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
cd "$SCRIPT_DIR"

# Helper: check compressed file is non-empty
check_nonempty() {
    local f="$1"
    if [ -f "$f" ] && [ -s "$f" ]; then
        return 0
    fi
    return 1
}

# Helper: verify gzip integrity
verify_gzip() {
    gunzip -t "$1" 2>/dev/null
}

# Helper: verify zlib integrity
verify_zlib() {
    python3 -c "import zlib; zlib.decompress(open('$1', 'rb').read())" 2>/dev/null
}

# Helper: check compression size (mirrors assert_file_compressed)
assert_compressed() {
    local original="$1"
    local compressed="$2"
    if [ ! -f "$original" ] || [ ! -f "$compressed" ]; then
        return 1
    fi
    local original_size compressed_size
    original_size=$(stat -c%s "$original")
    compressed_size=$(stat -c%s "$compressed")
    if [ "$compressed_size" -lt "$original_size" ]; then
        return 0
    elif [ "$original_size" -lt 100 ]; then
        return 0
    fi
    return 1
}

# Create small repetitive test input
make_small_input() {
    rm -f test_input.txt test_input.txt.gz test_input.txt.zlib test_input.txt.deflate
    echo "This is a test file for zopfli compression" > test_input.txt
    for i in {1..10}; do
        echo "This line is repeated multiple times to ensure compression works effectively. Line number: $i" >> test_input.txt
    done
}

# Create larger test input
make_large_input() {
    rm -f test_input.txt test_input.txt.gz test_input.txt.zlib test_input.txt.deflate
    echo "This is a test file for zopfli compression" > test_input.txt
    for i in {1..100}; do
        echo "Additional line $i with some random text to compress and make the file larger" >> test_input.txt
    done
}

ZOPFLI_BIN="./build/zopfli"
TRACE_ENV_PREFIX() {
    # echo env prefix for trace; caller uses it with eval
    local n="$1"
    echo "LD_PRELOAD=libtracer.so TRACE_OUTPUT=$PWD/flow_results/test${n}_trace.log"
}

############################
# Test 1: Basic gzip compression + size check + integrity check
############################
TEST_NUM=1
echo "Test $TEST_NUM started"
LOG="flow_results/test${TEST_NUM}_tmp.log"
: > "$LOG"
make_small_input >> "$LOG" 2>&1
rm -f test_input.txt.gz
LD_PRELOAD=libtracer.so TRACE_OUTPUT=$PWD/flow_results/test${TEST_NUM}_trace.log ./build/zopfli test_input.txt >> "$LOG" 2>&1
rc=$?
pass=1
if [ $rc -ne 0 ]; then pass=0; echo "zopfli command failed" >> "$LOG"; fi
if ! check_nonempty test_input.txt.gz; then pass=0; echo "output file missing or empty" >> "$LOG"; fi
if ! assert_compressed test_input.txt test_input.txt.gz; then pass=0; echo "compression check failed" >> "$LOG"; fi
if ! verify_gzip test_input.txt.gz; then pass=0; echo "gzip integrity check failed" >> "$LOG"; fi
if [ $pass -eq 1 ]; then
    mv "$LOG" flow_results/test${TEST_NUM}_success.log
    echo "Test $TEST_NUM passed"
else
    mv "$LOG" flow_results/test${TEST_NUM}_fail.log
    echo "Test $TEST_NUM failed" >&2
    failed=1
fi
echo "Test $TEST_NUM ended"

############################
# Test 2: Limited iterations (i1)
############################
TEST_NUM=2
echo "Test $TEST_NUM started"
LOG="flow_results/test${TEST_NUM}_tmp.log"
: > "$LOG"
make_small_input >> "$LOG" 2>&1
rm -f test_input.txt.gz
LD_PRELOAD=libtracer.so TRACE_OUTPUT=$PWD/flow_results/test${TEST_NUM}_trace.log ./build/zopfli --i1 test_input.txt >> "$LOG" 2>&1
rc=$?
pass=1
if [ $rc -ne 0 ]; then pass=0; fi
if ! check_nonempty test_input.txt.gz; then pass=0; fi
if [ $pass -eq 1 ]; then
    mv "$LOG" flow_results/test${TEST_NUM}_success.log
    echo "Test $TEST_NUM passed"
else
    mv "$LOG" flow_results/test${TEST_NUM}_fail.log
    echo "Test $TEST_NUM failed" >&2
    failed=1
fi
echo "Test $TEST_NUM ended"

############################
# Test 3: Block splitting enabled
############################
TEST_NUM=3
echo "Test $TEST_NUM started"
LOG="flow_results/test${TEST_NUM}_tmp.log"
: > "$LOG"
make_small_input >> "$LOG" 2>&1
rm -f test_input.txt.gz
LD_PRELOAD=libtracer.so TRACE_OUTPUT=$PWD/flow_results/test${TEST_NUM}_trace.log ./build/zopfli --blocksplitting=1 test_input.txt >> "$LOG" 2>&1
rc=$?
pass=1
if [ $rc -ne 0 ]; then pass=0; fi
if ! check_nonempty test_input.txt.gz; then pass=0; fi
if [ $pass -eq 1 ]; then
    mv "$LOG" flow_results/test${TEST_NUM}_success.log
    echo "Test $TEST_NUM passed"
else
    mv "$LOG" flow_results/test${TEST_NUM}_fail.log
    echo "Test $TEST_NUM failed" >&2
    failed=1
fi
echo "Test $TEST_NUM ended"

############################
# Test 4: Zlib format + size check + integrity check
############################
TEST_NUM=4
echo "Test $TEST_NUM started"
LOG="flow_results/test${TEST_NUM}_tmp.log"
: > "$LOG"
make_small_input >> "$LOG" 2>&1
rm -f test_input.txt.zlib
LD_PRELOAD=libtracer.so TRACE_OUTPUT=$PWD/flow_results/test${TEST_NUM}_trace.log ./build/zopfli --zlib test_input.txt >> "$LOG" 2>&1
rc=$?
pass=1
if [ $rc -ne 0 ]; then pass=0; echo "zopfli command failed" >> "$LOG"; fi
if ! check_nonempty test_input.txt.zlib; then pass=0; echo "output missing" >> "$LOG"; fi
if ! assert_compressed test_input.txt test_input.txt.zlib; then pass=0; echo "compression check failed" >> "$LOG"; fi
if ! verify_zlib test_input.txt.zlib; then pass=0; echo "zlib integrity check failed" >> "$LOG"; fi
if [ $pass -eq 1 ]; then
    mv "$LOG" flow_results/test${TEST_NUM}_success.log
    echo "Test $TEST_NUM passed"
else
    mv "$LOG" flow_results/test${TEST_NUM}_fail.log
    echo "Test $TEST_NUM failed" >&2
    failed=1
fi
echo "Test $TEST_NUM ended"

############################
# Test 5: Large file with verbose output
############################
TEST_NUM=5
echo "Test $TEST_NUM started"
LOG="flow_results/test${TEST_NUM}_tmp.log"
: > "$LOG"
make_large_input >> "$LOG" 2>&1
rm -f test_input.txt.gz
LD_PRELOAD=libtracer.so TRACE_OUTPUT=$PWD/flow_results/test${TEST_NUM}_trace.log ./build/zopfli --verbose test_input.txt >> "$LOG" 2>&1
rc=$?
pass=1
if [ $rc -ne 0 ]; then pass=0; fi
if ! check_nonempty test_input.txt.gz; then pass=0; fi
if [ $pass -eq 1 ]; then
    mv "$LOG" flow_results/test${TEST_NUM}_success.log
    echo "Test $TEST_NUM passed"
else
    mv "$LOG" flow_results/test${TEST_NUM}_fail.log
    echo "Test $TEST_NUM failed" >&2
    failed=1
fi
echo "Test $TEST_NUM ended"

############################
# Test 6: High iterations i100
############################
TEST_NUM=6
echo "Test $TEST_NUM started"
LOG="flow_results/test${TEST_NUM}_tmp.log"
: > "$LOG"
make_large_input >> "$LOG" 2>&1
rm -f test_input.txt.gz
LD_PRELOAD=libtracer.so TRACE_OUTPUT=$PWD/flow_results/test${TEST_NUM}_trace.log ./build/zopfli --i100 test_input.txt >> "$LOG" 2>&1
rc=$?
pass=1
if [ $rc -ne 0 ]; then pass=0; fi
if ! check_nonempty test_input.txt.gz; then pass=0; fi
if [ $pass -eq 1 ]; then
    mv "$LOG" flow_results/test${TEST_NUM}_success.log
    echo "Test $TEST_NUM passed"
else
    mv "$LOG" flow_results/test${TEST_NUM}_fail.log
    echo "Test $TEST_NUM failed" >&2
    failed=1
fi
echo "Test $TEST_NUM ended"

############################
# Test 7: Combined options (i50 + blocksplitting)
############################
TEST_NUM=7
echo "Test $TEST_NUM started"
LOG="flow_results/test${TEST_NUM}_tmp.log"
: > "$LOG"
make_large_input >> "$LOG" 2>&1
rm -f test_input.txt.gz
LD_PRELOAD=libtracer.so TRACE_OUTPUT=$PWD/flow_results/test${TEST_NUM}_trace.log ./build/zopfli --i50 --blocksplitting=1 test_input.txt >> "$LOG" 2>&1
rc=$?
pass=1
if [ $rc -ne 0 ]; then pass=0; fi
if ! check_nonempty test_input.txt.gz; then pass=0; fi
if [ $pass -eq 1 ]; then
    mv "$LOG" flow_results/test${TEST_NUM}_success.log
    echo "Test $TEST_NUM passed"
else
    mv "$LOG" flow_results/test${TEST_NUM}_fail.log
    echo "Test $TEST_NUM failed" >&2
    failed=1
fi
echo "Test $TEST_NUM ended"

############################
# Test 8: Block splitting max=10
############################
TEST_NUM=8
echo "Test $TEST_NUM started"
LOG="flow_results/test${TEST_NUM}_tmp.log"
: > "$LOG"
make_large_input >> "$LOG" 2>&1
rm -f test_input.txt.gz
LD_PRELOAD=libtracer.so TRACE_OUTPUT=$PWD/flow_results/test${TEST_NUM}_trace.log ./build/zopfli --blocksplittingmax=10 test_input.txt >> "$LOG" 2>&1
rc=$?
pass=1
if [ $rc -ne 0 ]; then pass=0; fi
if ! check_nonempty test_input.txt.gz; then pass=0; fi
if [ $pass -eq 1 ]; then
    mv "$LOG" flow_results/test${TEST_NUM}_success.log
    echo "Test $TEST_NUM passed"
else
    mv "$LOG" flow_results/test${TEST_NUM}_fail.log
    echo "Test $TEST_NUM failed" >&2
    failed=1
fi
echo "Test $TEST_NUM ended"

############################
# Test 9: Multiple format - zlib (large input)
############################
TEST_NUM=9
echo "Test $TEST_NUM started"
LOG="flow_results/test${TEST_NUM}_tmp.log"
: > "$LOG"
make_large_input >> "$LOG" 2>&1
rm -f test_input.txt.zlib
LD_PRELOAD=libtracer.so TRACE_OUTPUT=$PWD/flow_results/test${TEST_NUM}_trace.log ./build/zopfli --zlib test_input.txt >> "$LOG" 2>&1
rc=$?
pass=1
if [ $rc -ne 0 ]; then pass=0; fi
if ! check_nonempty test_input.txt.zlib; then pass=0; fi
if [ $pass -eq 1 ]; then
    mv "$LOG" flow_results/test${TEST_NUM}_success.log
    echo "Test $TEST_NUM passed"
else
    mv "$LOG" flow_results/test${TEST_NUM}_fail.log
    echo "Test $TEST_NUM failed" >&2
    failed=1
fi
echo "Test $TEST_NUM ended"

############################
# Test 10: Empty file compression + gzip integrity
############################
TEST_NUM=10
echo "Test $TEST_NUM started"
LOG="flow_results/test${TEST_NUM}_tmp.log"
: > "$LOG"
rm -f empty_file.txt empty_file.txt.gz
echo -n "" > empty_file.txt
LD_PRELOAD=libtracer.so TRACE_OUTPUT=$PWD/flow_results/test${TEST_NUM}_trace.log ./build/zopfli empty_file.txt >> "$LOG" 2>&1
rc=$?
pass=1
if [ $rc -ne 0 ]; then pass=0; echo "zopfli failed" >> "$LOG"; fi
if ! check_nonempty empty_file.txt.gz; then pass=0; echo "empty_file.txt.gz missing or empty" >> "$LOG"; fi
if ! verify_gzip empty_file.txt.gz; then pass=0; echo "gzip integrity failed" >> "$LOG"; fi
rm -f empty_file.txt empty_file.txt.gz
if [ $pass -eq 1 ]; then
    mv "$LOG" flow_results/test${TEST_NUM}_success.log
    echo "Test $TEST_NUM passed"
else
    mv "$LOG" flow_results/test${TEST_NUM}_fail.log
    echo "Test $TEST_NUM failed" >&2
    failed=1
fi
echo "Test $TEST_NUM ended"

############################
# Test 11: Special characters file
############################
TEST_NUM=11
echo "Test $TEST_NUM started"
LOG="flow_results/test${TEST_NUM}_tmp.log"
: > "$LOG"
rm -f special_chars.txt special_chars.txt.gz
echo "Special chars: !@#\$%^&*()_+{}|:<>?[]\\;'\",./" > special_chars.txt
LD_PRELOAD=libtracer.so TRACE_OUTPUT=$PWD/flow_results/test${TEST_NUM}_trace.log ./build/zopfli special_chars.txt >> "$LOG" 2>&1
rc=$?
pass=1
if [ $rc -ne 0 ]; then pass=0; fi
if ! check_nonempty special_chars.txt.gz; then pass=0; fi
rm -f special_chars.txt special_chars.txt.gz
if [ $pass -eq 1 ]; then
    mv "$LOG" flow_results/test${TEST_NUM}_success.log
    echo "Test $TEST_NUM passed"
else
    mv "$LOG" flow_results/test${TEST_NUM}_fail.log
    echo "Test $TEST_NUM failed" >&2
    failed=1
fi
echo "Test $TEST_NUM ended"

############################
# Test 12: Block splitting disabled
############################
TEST_NUM=12
echo "Test $TEST_NUM started"
LOG="flow_results/test${TEST_NUM}_tmp.log"
: > "$LOG"
make_large_input >> "$LOG" 2>&1
rm -f test_input.txt.gz
LD_PRELOAD=libtracer.so TRACE_OUTPUT=$PWD/flow_results/test${TEST_NUM}_trace.log ./build/zopfli --i1 --blocksplitting=0 test_input.txt >> "$LOG" 2>&1
rc=$?
pass=1
if [ $rc -ne 0 ]; then pass=0; fi
if ! check_nonempty test_input.txt.gz; then pass=0; fi
if [ $pass -eq 1 ]; then
    mv "$LOG" flow_results/test${TEST_NUM}_success.log
    echo "Test $TEST_NUM passed"
else
    mv "$LOG" flow_results/test${TEST_NUM}_fail.log
    echo "Test $TEST_NUM failed" >&2
    failed=1
fi
echo "Test $TEST_NUM ended"

############################
# Test 13: Zlib with i1
############################
TEST_NUM=13
echo "Test $TEST_NUM started"
LOG="flow_results/test${TEST_NUM}_tmp.log"
: > "$LOG"
make_large_input >> "$LOG" 2>&1
rm -f test_input.txt.zlib
LD_PRELOAD=libtracer.so TRACE_OUTPUT=$PWD/flow_results/test${TEST_NUM}_trace.log ./build/zopfli --i1 --zlib test_input.txt >> "$LOG" 2>&1
rc=$?
pass=1
if [ $rc -ne 0 ]; then pass=0; fi
if ! check_nonempty test_input.txt.zlib; then pass=0; fi
if [ $pass -eq 1 ]; then
    mv "$LOG" flow_results/test${TEST_NUM}_success.log
    echo "Test $TEST_NUM passed"
else
    mv "$LOG" flow_results/test${TEST_NUM}_fail.log
    echo "Test $TEST_NUM failed" >&2
    failed=1
fi
echo "Test $TEST_NUM ended"

############################
# Test 14: Explicit gzip format
############################
TEST_NUM=14
echo "Test $TEST_NUM started"
LOG="flow_results/test${TEST_NUM}_tmp.log"
: > "$LOG"
make_large_input >> "$LOG" 2>&1
rm -f test_input.txt.gz
LD_PRELOAD=libtracer.so TRACE_OUTPUT=$PWD/flow_results/test${TEST_NUM}_trace.log ./build/zopfli --gzip test_input.txt >> "$LOG" 2>&1
rc=$?
pass=1
if [ $rc -ne 0 ]; then pass=0; fi
if ! check_nonempty test_input.txt.gz; then pass=0; fi
if [ $pass -eq 1 ]; then
    mv "$LOG" flow_results/test${TEST_NUM}_success.log
    echo "Test $TEST_NUM passed"
else
    mv "$LOG" flow_results/test${TEST_NUM}_fail.log
    echo "Test $TEST_NUM failed" >&2
    failed=1
fi
echo "Test $TEST_NUM ended"

############################
# Test 15: Verbose with block splitting
############################
TEST_NUM=15
echo "Test $TEST_NUM started"
LOG="flow_results/test${TEST_NUM}_tmp.log"
: > "$LOG"
make_large_input >> "$LOG" 2>&1
rm -f test_input.txt.gz
LD_PRELOAD=libtracer.so TRACE_OUTPUT=$PWD/flow_results/test${TEST_NUM}_trace.log ./build/zopfli --verbose --blocksplitting=1 test_input.txt >> "$LOG" 2>&1
rc=$?
pass=1
if [ $rc -ne 0 ]; then pass=0; fi
if ! check_nonempty test_input.txt.gz; then pass=0; fi
if [ $pass -eq 1 ]; then
    mv "$LOG" flow_results/test${TEST_NUM}_success.log
    echo "Test $TEST_NUM passed"
else
    mv "$LOG" flow_results/test${TEST_NUM}_fail.log
    echo "Test $TEST_NUM failed" >&2
    failed=1
fi
echo "Test $TEST_NUM ended"

############################
# Test 16: Verbose with zlib
############################
TEST_NUM=16
echo "Test $TEST_NUM started"
LOG="flow_results/test${TEST_NUM}_tmp.log"
: > "$LOG"
make_large_input >> "$LOG" 2>&1
rm -f test_input.txt.zlib
LD_PRELOAD=libtracer.so TRACE_OUTPUT=$PWD/flow_results/test${TEST_NUM}_trace.log ./build/zopfli --verbose --zlib test_input.txt >> "$LOG" 2>&1
rc=$?
pass=1
if [ $rc -ne 0 ]; then pass=0; fi
if ! check_nonempty test_input.txt.zlib; then pass=0; fi
if [ $pass -eq 1 ]; then
    mv "$LOG" flow_results/test${TEST_NUM}_success.log
    echo "Test $TEST_NUM passed"
else
    mv "$LOG" flow_results/test${TEST_NUM}_fail.log
    echo "Test $TEST_NUM failed" >&2
    failed=1
fi
echo "Test $TEST_NUM ended"

############################
# Test 17: Verbose with gzip
############################
TEST_NUM=17
echo "Test $TEST_NUM started"
LOG="flow_results/test${TEST_NUM}_tmp.log"
: > "$LOG"
make_large_input >> "$LOG" 2>&1
rm -f test_input.txt.gz
LD_PRELOAD=libtracer.so TRACE_OUTPUT=$PWD/flow_results/test${TEST_NUM}_trace.log ./build/zopfli --verbose --gzip test_input.txt >> "$LOG" 2>&1
rc=$?
pass=1
if [ $rc -ne 0 ]; then pass=0; fi
if ! check_nonempty test_input.txt.gz; then pass=0; fi
if [ $pass -eq 1 ]; then
    mv "$LOG" flow_results/test${TEST_NUM}_success.log
    echo "Test $TEST_NUM passed"
else
    mv "$LOG" flow_results/test${TEST_NUM}_fail.log
    echo "Test $TEST_NUM failed" >&2
    failed=1
fi
echo "Test $TEST_NUM ended"

############################
# Test 18: Verbose with i10 and blocksplitting
############################
TEST_NUM=18
echo "Test $TEST_NUM started"
LOG="flow_results/test${TEST_NUM}_tmp.log"
: > "$LOG"
make_large_input >> "$LOG" 2>&1
rm -f test_input.txt.gz
LD_PRELOAD=libtracer.so TRACE_OUTPUT=$PWD/flow_results/test${TEST_NUM}_trace.log ./build/zopfli --verbose --i10 --blocksplitting=1 test_input.txt >> "$LOG" 2>&1
rc=$?
pass=1
if [ $rc -ne 0 ]; then pass=0; fi
if ! check_nonempty test_input.txt.gz; then pass=0; fi
if [ $pass -eq 1 ]; then
    mv "$LOG" flow_results/test${TEST_NUM}_success.log
    echo "Test $TEST_NUM passed"
else
    mv "$LOG" flow_results/test${TEST_NUM}_fail.log
    echo "Test $TEST_NUM failed" >&2
    failed=1
fi
echo "Test $TEST_NUM ended"

############################
# Test 19: High iterations i15
############################
TEST_NUM=19
echo "Test $TEST_NUM started"
LOG="flow_results/test${TEST_NUM}_tmp.log"
: > "$LOG"
make_large_input >> "$LOG" 2>&1
rm -f test_input.txt.gz
LD_PRELOAD=libtracer.so TRACE_OUTPUT=$PWD/flow_results/test${TEST_NUM}_trace.log ./build/zopfli --verbose --i15 test_input.txt >> "$LOG" 2>&1
rc=$?
pass=1
if [ $rc -ne 0 ]; then pass=0; fi
if ! check_nonempty test_input.txt.gz; then pass=0; fi
if [ $pass -eq 1 ]; then
    mv "$LOG" flow_results/test${TEST_NUM}_success.log
    echo "Test $TEST_NUM passed"
else
    mv "$LOG" flow_results/test${TEST_NUM}_fail.log
    echo "Test $TEST_NUM failed" >&2
    failed=1
fi
echo "Test $TEST_NUM ended"

############################
# Test 20: First compression
############################
TEST_NUM=20
echo "Test $TEST_NUM started"
LOG="flow_results/test${TEST_NUM}_tmp.log"
: > "$LOG"
make_large_input >> "$LOG" 2>&1
rm -f test_input.txt.gz
LD_PRELOAD=libtracer.so TRACE_OUTPUT=$PWD/flow_results/test${TEST_NUM}_trace.log ./build/zopfli test_input.txt >> "$LOG" 2>&1
rc=$?
pass=1
if [ $rc -ne 0 ]; then pass=0; fi
if ! check_nonempty test_input.txt.gz; then pass=0; fi
if [ $pass -eq 1 ]; then
    mv "$LOG" flow_results/test${TEST_NUM}_success.log
    echo "Test $TEST_NUM passed"
else
    mv "$LOG" flow_results/test${TEST_NUM}_fail.log
    echo "Test $TEST_NUM failed" >&2
    failed=1
fi
echo "Test $TEST_NUM ended"

############################
# Test 21: Second compression with different options
############################
TEST_NUM=21
echo "Test $TEST_NUM started"
LOG="flow_results/test${TEST_NUM}_tmp.log"
: > "$LOG"
make_large_input >> "$LOG" 2>&1
rm -f test_input.txt.gz
LD_PRELOAD=libtracer.so TRACE_OUTPUT=$PWD/flow_results/test${TEST_NUM}_trace.log ./build/zopfli --i1 test_input.txt >> "$LOG" 2>&1
rc=$?
pass=1
if [ $rc -ne 0 ]; then pass=0; fi
if ! check_nonempty test_input.txt.gz; then pass=0; fi
if [ $pass -eq 1 ]; then
    mv "$LOG" flow_results/test${TEST_NUM}_success.log
    echo "Test $TEST_NUM passed"
else
    mv "$LOG" flow_results/test${TEST_NUM}_fail.log
    echo "Test $TEST_NUM failed" >&2
    failed=1
fi
echo "Test $TEST_NUM ended"

# Final cleanup
rm -f test_input.txt test_input.txt.gz test_input.txt.zlib test_input.txt.deflate
rm -f empty_file.txt empty_file.txt.gz
rm -f special_chars.txt special_chars.txt.gz

exit $failed

