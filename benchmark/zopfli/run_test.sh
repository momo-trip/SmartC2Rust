
#!/bin/bash

# Reformed test cases for zopfli

failed=0

# Create results directory
mkdir -p genifai_results

# Clean up any existing test files
rm -f test_input.txt test_input.txt.gz test_input.txt.zlib test_input.txt.deflate
rm -f empty_file.txt empty_file.txt.gz
rm -f special_chars.txt special_chars.txt.gz

# Create initial test file (small)
echo "This is a test file for zopfli compression" > test_input.txt
for i in $(seq 1 10); do
    echo "This line is repeated multiple times to ensure compression works effectively. Line number: $i" >> test_input.txt
done

BINDIR=test_binaries

############################################
# Test 1: Basic gzip compression
############################################
test_num=1
echo "Test ${test_num} started"
log=""
rm -f test_input.txt.gz
output=$(LD_PRELOAD=libtracer.so TRACE_OUTPUT=$PWD/genifai_results/test${test_num}_trace.log ./${BINDIR}/zopfli_t${test_num} test_input.txt 2>&1)
result=$?
log+="Command output: ${output}"$'\n'
if [ $result -eq 0 ] && [ -f "test_input.txt.gz" ] && [ -s "test_input.txt.gz" ]; then
    echo "Test ${test_num} passed"
    echo "${log}" > genifai_results/test${test_num}_success.log
else
    echo "Test ${test_num} failed"
    echo "Test ${test_num} failed" >&2
    echo "${log}" > genifai_results/test${test_num}_fail.log
    failed=1
fi
echo "Test ${test_num} ended"

############################################
# Test 2: Compression check for basic gzip
############################################
test_num=2
echo "Test ${test_num} started"
log=""
if [ -f "test_input.txt.gz" ] && [ -f "test_input.txt" ]; then
    original_size=$(stat -c%s "test_input.txt")
    compressed_size=$(stat -c%s "test_input.txt.gz")
    log+="Original: ${original_size}B, Compressed: ${compressed_size}B"$'\n'
    if [ "$compressed_size" -lt "$original_size" ] || [ "$original_size" -lt 100 ]; then
        echo "Test ${test_num} passed"
        echo "${log}" > genifai_results/test${test_num}_success.log
    else
        echo "Test ${test_num} failed"
        echo "Test ${test_num} failed" >&2
        log+="Large file not compressed properly"$'\n'
        echo "${log}" > genifai_results/test${test_num}_fail.log
        failed=1
    fi
else
    echo "Test ${test_num} failed"
    echo "Test ${test_num} failed" >&2
    log+="Files not found for compression check"$'\n'
    echo "${log}" > genifai_results/test${test_num}_fail.log
    failed=1
fi
echo "Test ${test_num} ended"

############################################
# Test 3: Integrity check for basic gzip
############################################
test_num=3
echo "Test ${test_num} started"
log=""
if [ -f "test_input.txt.gz" ]; then
    gunzip_output=$(gunzip -t test_input.txt.gz 2>&1)
    gunzip_result=$?
    log+="gunzip -t result: ${gunzip_result}, output: ${gunzip_output}"$'\n'
    if [ $gunzip_result -eq 0 ]; then
        echo "Test ${test_num} passed"
        echo "${log}" > genifai_results/test${test_num}_success.log
    else
        echo "Test ${test_num} failed"
        echo "Test ${test_num} failed" >&2
        echo "${log}" > genifai_results/test${test_num}_fail.log
        failed=1
    fi
else
    echo "Test ${test_num} failed"
    echo "Test ${test_num} failed" >&2
    log+="test_input.txt.gz not found for integrity check"$'\n'
    echo "${log}" > genifai_results/test${test_num}_fail.log
    failed=1
fi
echo "Test ${test_num} ended"

############################################
# Test 4: Limited iterations (i1)
############################################
test_num=4
echo "Test ${test_num} started"
log=""
rm -f test_input.txt.gz
output=$(LD_PRELOAD=libtracer.so TRACE_OUTPUT=$PWD/genifai_results/test${test_num}_trace.log ./${BINDIR}/zopfli_t${test_num} --i1 test_input.txt 2>&1)
result=$?
log+="Command output: ${output}"$'\n'
if [ $result -eq 0 ] && [ -f "test_input.txt.gz" ] && [ -s "test_input.txt.gz" ]; then
    echo "Test ${test_num} passed"
    echo "${log}" > genifai_results/test${test_num}_success.log
else
    echo "Test ${test_num} failed"
    echo "Test ${test_num} failed" >&2
    echo "${log}" > genifai_results/test${test_num}_fail.log
    failed=1
fi
echo "Test ${test_num} ended"

############################################
# Test 5: Block splitting enabled
############################################
test_num=5
echo "Test ${test_num} started"
log=""
rm -f test_input.txt.gz
output=$(LD_PRELOAD=libtracer.so TRACE_OUTPUT=$PWD/genifai_results/test${test_num}_trace.log ./${BINDIR}/zopfli_t${test_num} --blocksplitting=1 test_input.txt 2>&1)
result=$?
log+="Command output: ${output}"$'\n'
if [ $result -eq 0 ] && [ -f "test_input.txt.gz" ] && [ -s "test_input.txt.gz" ]; then
    echo "Test ${test_num} passed"
    echo "${log}" > genifai_results/test${test_num}_success.log
else
    echo "Test ${test_num} failed"
    echo "Test ${test_num} failed" >&2
    echo "${log}" > genifai_results/test${test_num}_fail.log
    failed=1
fi
echo "Test ${test_num} ended"

############################################
# Test 6: Zlib format
############################################
test_num=6
echo "Test ${test_num} started"
log=""
rm -f test_input.txt.zlib
output=$(LD_PRELOAD=libtracer.so TRACE_OUTPUT=$PWD/genifai_results/test${test_num}_trace.log ./${BINDIR}/zopfli_t${test_num} --zlib test_input.txt 2>&1)
result=$?
log+="Command output: ${output}"$'\n'
if [ $result -eq 0 ] && [ -f "test_input.txt.zlib" ] && [ -s "test_input.txt.zlib" ]; then
    echo "Test ${test_num} passed"
    echo "${log}" > genifai_results/test${test_num}_success.log
else
    echo "Test ${test_num} failed"
    echo "Test ${test_num} failed" >&2
    echo "${log}" > genifai_results/test${test_num}_fail.log
    failed=1
fi
echo "Test ${test_num} ended"

############################################
# Test 7: Compression check for zlib
############################################
test_num=7
echo "Test ${test_num} started"
log=""
if [ -f "test_input.txt.zlib" ] && [ -f "test_input.txt" ]; then
    original_size=$(stat -c%s "test_input.txt")
    compressed_size=$(stat -c%s "test_input.txt.zlib")
    log+="Original: ${original_size}B, Compressed: ${compressed_size}B"$'\n'
    if [ "$compressed_size" -lt "$original_size" ] || [ "$original_size" -lt 100 ]; then
        echo "Test ${test_num} passed"
        echo "${log}" > genifai_results/test${test_num}_success.log
    else
        echo "Test ${test_num} failed"
        echo "Test ${test_num} failed" >&2
        log+="Large file not compressed properly"$'\n'
        echo "${log}" > genifai_results/test${test_num}_fail.log
        failed=1
    fi
else
    echo "Test ${test_num} failed"
    echo "Test ${test_num} failed" >&2
    log+="Files not found for compression check"$'\n'
    echo "${log}" > genifai_results/test${test_num}_fail.log
    failed=1
fi
echo "Test ${test_num} ended"

############################################
# Test 8: Integrity check for zlib
############################################
test_num=8
echo "Test ${test_num} started"
log=""
if [ -f "test_input.txt.zlib" ]; then
    python_output=$(python3 -c "import zlib; zlib.decompress(open('test_input.txt.zlib', 'rb').read())" 2>&1)
    python_result=$?
    log+="python3 zlib check result: ${python_result}, output: ${python_output}"$'\n'
    if [ $python_result -eq 0 ]; then
        echo "Test ${test_num} passed"
        echo "${log}" > genifai_results/test${test_num}_success.log
    else
        echo "Test ${test_num} failed"
        echo "Test ${test_num} failed" >&2
        echo "${log}" > genifai_results/test${test_num}_fail.log
        failed=1
    fi
else
    echo "Test ${test_num} failed"
    echo "Test ${test_num} failed" >&2
    log+="test_input.txt.zlib not found for integrity check"$'\n'
    echo "${log}" > genifai_results/test${test_num}_fail.log
    failed=1
fi
echo "Test ${test_num} ended"

############################################
# Recreate larger test file for remaining tests
############################################
rm -f test_input.txt test_input.txt.gz test_input.txt.zlib test_input.txt.deflate
echo "This is a test file for zopfli compression" > test_input.txt
for i in $(seq 1 100); do
    echo "Additional line $i with some random text to compress and make the file larger" >> test_input.txt
done

############################################
# Test 9: Large file with verbose output
############################################
test_num=9
echo "Test ${test_num} started"
log=""
rm -f test_input.txt.gz
output=$(LD_PRELOAD=libtracer.so TRACE_OUTPUT=$PWD/genifai_results/test${test_num}_trace.log ./${BINDIR}/zopfli_t${test_num} --verbose test_input.txt 2>&1)
result=$?
log+="Command output: ${output}"$'\n'
if [ $result -eq 0 ] && [ -f "test_input.txt.gz" ] && [ -s "test_input.txt.gz" ]; then
    echo "Test ${test_num} passed"
    echo "${log}" > genifai_results/test${test_num}_success.log
else
    echo "Test ${test_num} failed"
    echo "Test ${test_num} failed" >&2
    echo "${log}" > genifai_results/test${test_num}_fail.log
    failed=1
fi
echo "Test ${test_num} ended"

############################################
# Test 10: High iterations (i100)
############################################
test_num=10
echo "Test ${test_num} started"
log=""
rm -f test_input.txt.gz
output=$(LD_PRELOAD=libtracer.so TRACE_OUTPUT=$PWD/genifai_results/test${test_num}_trace.log ./${BINDIR}/zopfli_t${test_num} --i100 test_input.txt 2>&1)
result=$?
log+="Command output: ${output}"$'\n'
if [ $result -eq 0 ] && [ -f "test_input.txt.gz" ] && [ -s "test_input.txt.gz" ]; then
    echo "Test ${test_num} passed"
    echo "${log}" > genifai_results/test${test_num}_success.log
else
    echo "Test ${test_num} failed"
    echo "Test ${test_num} failed" >&2
    echo "${log}" > genifai_results/test${test_num}_fail.log
    failed=1
fi
echo "Test ${test_num} ended"

############################################
# Test 11: Combined options (i50 + blocksplitting)
############################################
test_num=11
echo "Test ${test_num} started"
log=""
rm -f test_input.txt.gz
output=$(LD_PRELOAD=libtracer.so TRACE_OUTPUT=$PWD/genifai_results/test${test_num}_trace.log ./${BINDIR}/zopfli_t${test_num} --i50 --blocksplitting=1 test_input.txt 2>&1)
result=$?
log+="Command output: ${output}"$'\n'
if [ $result -eq 0 ] && [ -f "test_input.txt.gz" ] && [ -s "test_input.txt.gz" ]; then
    echo "Test ${test_num} passed"
    echo "${log}" > genifai_results/test${test_num}_success.log
else
    echo "Test ${test_num} failed"
    echo "Test ${test_num} failed" >&2
    echo "${log}" > genifai_results/test${test_num}_fail.log
    failed=1
fi
echo "Test ${test_num} ended"

############################################
# Test 12: Block splitting max=10
############################################
test_num=12
echo "Test ${test_num} started"
log=""
rm -f test_input.txt.gz
output=$(LD_PRELOAD=libtracer.so TRACE_OUTPUT=$PWD/genifai_results/test${test_num}_trace.log ./${BINDIR}/zopfli_t${test_num} --blocksplittingmax=10 test_input.txt 2>&1)
result=$?
log+="Command output: ${output}"$'\n'
if [ $result -eq 0 ] && [ -f "test_input.txt.gz" ] && [ -s "test_input.txt.gz" ]; then
    echo "Test ${test_num} passed"
    echo "${log}" > genifai_results/test${test_num}_success.log
else
    echo "Test ${test_num} failed"
    echo "Test ${test_num} failed" >&2
    echo "${log}" > genifai_results/test${test_num}_fail.log
    failed=1
fi
echo "Test ${test_num} ended"

############################################
# Test 13: Multiple format - zlib
############################################
test_num=13
echo "Test ${test_num} started"
log=""
rm -f test_input.txt.zlib
output=$(LD_PRELOAD=libtracer.so TRACE_OUTPUT=$PWD/genifai_results/test${test_num}_trace.log ./${BINDIR}/zopfli_t${test_num} --zlib test_input.txt 2>&1)
result=$?
log+="Command output: ${output}"$'\n'
if [ $result -eq 0 ] && [ -f "test_input.txt.zlib" ] && [ -s "test_input.txt.zlib" ]; then
    echo "Test ${test_num} passed"
    echo "${log}" > genifai_results/test${test_num}_success.log
else
    echo "Test ${test_num} failed"
    echo "Test ${test_num} failed" >&2
    echo "${log}" > genifai_results/test${test_num}_fail.log
    failed=1
fi
echo "Test ${test_num} ended"

############################################
# Test 14: Special characters file
############################################
test_num=14
echo "Test ${test_num} started"
log=""
echo 'Special chars: !@#$%^&*()_+{}|:<>?[]\;'\''",./' > special_chars.txt
rm -f special_chars.txt.gz
output=$(LD_PRELOAD=libtracer.so TRACE_OUTPUT=$PWD/genifai_results/test${test_num}_trace.log ./${BINDIR}/zopfli_t${test_num} special_chars.txt 2>&1)
result=$?
log+="Command output: ${output}"$'\n'
if [ $result -eq 0 ] && [ -f "special_chars.txt.gz" ] && [ -s "special_chars.txt.gz" ]; then
    echo "Test ${test_num} passed"
    echo "${log}" > genifai_results/test${test_num}_success.log
else
    echo "Test ${test_num} failed"
    echo "Test ${test_num} failed" >&2
    echo "${log}" > genifai_results/test${test_num}_fail.log
    failed=1
fi
echo "Test ${test_num} ended"

############################################
# Test 15: Block splitting disabled
############################################
test_num=15
echo "Test ${test_num} started"
log=""
rm -f test_input.txt.gz
output=$(LD_PRELOAD=libtracer.so TRACE_OUTPUT=$PWD/genifai_results/test${test_num}_trace.log ./${BINDIR}/zopfli_t${test_num} --i1 --blocksplitting=0 test_input.txt 2>&1)
result=$?
log+="Command output: ${output}"$'\n'
if [ $result -eq 0 ] && [ -f "test_input.txt.gz" ] && [ -s "test_input.txt.gz" ]; then
    echo "Test ${test_num} passed"
    echo "${log}" > genifai_results/test${test_num}_success.log
else
    echo "Test ${test_num} failed"
    echo "Test ${test_num} failed" >&2
    echo "${log}" > genifai_results/test${test_num}_fail.log
    failed=1
fi
echo "Test ${test_num} ended"

############################################
# Test 16: Zlib with i1
############################################
test_num=16
echo "Test ${test_num} started"
log=""
rm -f test_input.txt.zlib
output=$(LD_PRELOAD=libtracer.so TRACE_OUTPUT=$PWD/genifai_results/test${test_num}_trace.log ./${BINDIR}/zopfli_t${test_num} --i1 --zlib test_input.txt 2>&1)
result=$?
log+="Command output: ${output}"$'\n'
if [ $result -eq 0 ] && [ -f "test_input.txt.zlib" ] && [ -s "test_input.txt.zlib" ]; then
    echo "Test ${test_num} passed"
    echo "${log}" > genifai_results/test${test_num}_success.log
else
    echo "Test ${test_num} failed"
    echo "Test ${test_num} failed" >&2
    echo "${log}" > genifai_results/test${test_num}_fail.log
    failed=1
fi
echo "Test ${test_num} ended"

############################################
# Test 17: Explicit gzip format
############################################
test_num=17
echo "Test ${test_num} started"
log=""
rm -f test_input.txt.gz
output=$(LD_PRELOAD=libtracer.so TRACE_OUTPUT=$PWD/genifai_results/test${test_num}_trace.log ./${BINDIR}/zopfli_t${test_num} --gzip test_input.txt 2>&1)
result=$?
log+="Command output: ${output}"$'\n'
if [ $result -eq 0 ] && [ -f "test_input.txt.gz" ] && [ -s "test_input.txt.gz" ]; then
    echo "Test ${test_num} passed"
    echo "${log}" > genifai_results/test${test_num}_success.log
else
    echo "Test ${test_num} failed"
    echo "Test ${test_num} failed" >&2
    echo "${log}" > genifai_results/test${test_num}_fail.log
    failed=1
fi
echo "Test ${test_num} ended"

############################################
# Test 18: Verbose with block splitting
############################################
test_num=18
echo "Test ${test_num} started"
log=""
rm -f test_input.txt.gz
output=$(LD_PRELOAD=libtracer.so TRACE_OUTPUT=$PWD/genifai_results/test${test_num}_trace.log ./${BINDIR}/zopfli_t${test_num} --verbose --blocksplitting=1 test_input.txt 2>&1)
result=$?
log+="Command output: ${output}"$'\n'
if [ $result -eq 0 ] && [ -f "test_input.txt.gz" ] && [ -s "test_input.txt.gz" ]; then
    echo "Test ${test_num} passed"
    echo "${log}" > genifai_results/test${test_num}_success.log
else
    echo "Test ${test_num} failed"
    echo "Test ${test_num} failed" >&2
    echo "${log}" > genifai_results/test${test_num}_fail.log
    failed=1
fi
echo "Test ${test_num} ended"

############################################
# Test 19: Verbose with zlib
############################################
test_num=19
echo "Test ${test_num} started"
log=""
rm -f test_input.txt.zlib
output=$(LD_PRELOAD=libtracer.so TRACE_OUTPUT=$PWD/genifai_results/test${test_num}_trace.log ./${BINDIR}/zopfli_t${test_num} --verbose --zlib test_input.txt 2>&1)
result=$?
log+="Command output: ${output}"$'\n'
if [ $result -eq 0 ] && [ -f "test_input.txt.zlib" ] && [ -s "test_input.txt.zlib" ]; then
    echo "Test ${test_num} passed"
    echo "${log}" > genifai_results/test${test_num}_success.log
else
    echo "Test ${test_num} failed"
    echo "Test ${test_num} failed" >&2
    echo "${log}" > genifai_results/test${test_num}_fail.log
    failed=1
fi
echo "Test ${test_num} ended"

############################################
# Test 20: Verbose with gzip
############################################
test_num=20
echo "Test ${test_num} started"
log=""
rm -f test_input.txt.gz
output=$(LD_PRELOAD=libtracer.so TRACE_OUTPUT=$PWD/genifai_results/test${test_num}_trace.log ./${BINDIR}/zopfli_t${test_num} --verbose --gzip test_input.txt 2>&1)
result=$?
log+="Command output: ${output}"$'\n'
if [ $result -eq 0 ] && [ -f "test_input.txt.gz" ] && [ -s "test_input.txt.gz" ]; then
    echo "Test ${test_num} passed"
    echo "${log}" > genifai_results/test${test_num}_success.log
else
    echo "Test ${test_num} failed"
    echo "Test ${test_num} failed" >&2
    echo "${log}" > genifai_results/test${test_num}_fail.log
    failed=1
fi
echo "Test ${test_num} ended"

############################################
# Test 21: Verbose with i10 and blocksplitting
############################################
test_num=21
echo "Test ${test_num} started"
log=""
rm -f test_input.txt.gz
output=$(LD_PRELOAD=libtracer.so TRACE_OUTPUT=$PWD/genifai_results/test${test_num}_trace.log ./${BINDIR}/zopfli_t${test_num} --verbose --i10 --blocksplitting=1 test_input.txt 2>&1)
result=$?
log+="Command output: ${output}"$'\n'
if [ $result -eq 0 ] && [ -f "test_input.txt.gz" ] && [ -s "test_input.txt.gz" ]; then
    echo "Test ${test_num} passed"
    echo "${log}" > genifai_results/test${test_num}_success.log
else
    echo "Test ${test_num} failed"
    echo "Test ${test_num} failed" >&2
    echo "${log}" > genifai_results/test${test_num}_fail.log
    failed=1
fi
echo "Test ${test_num} ended"

############################################
# Test 22: High iterations i15
############################################
test_num=22
echo "Test ${test_num} started"
log=""
rm -f test_input.txt.gz
output=$(LD_PRELOAD=libtracer.so TRACE_OUTPUT=$PWD/genifai_results/test${test_num}_trace.log ./${BINDIR}/zopfli_t${test_num} --verbose --i15 test_input.txt 2>&1)
result=$?
log+="Command output: ${output}"$'\n'
if [ $result -eq 0 ] && [ -f "test_input.txt.gz" ] && [ -s "test_input.txt.gz" ]; then
    echo "Test ${test_num} passed"
    echo "${log}" > genifai_results/test${test_num}_success.log
else
    echo "Test ${test_num} failed"
    echo "Test ${test_num} failed" >&2
    echo "${log}" > genifai_results/test${test_num}_fail.log
    failed=1
fi
echo "Test ${test_num} ended"

############################################
# Test 23: First compression (multiple compressions)
############################################
test_num=23
echo "Test ${test_num} started"
log=""
rm -f test_input.txt.gz
output=$(LD_PRELOAD=libtracer.so TRACE_OUTPUT=$PWD/genifai_results/test${test_num}_trace.log ./${BINDIR}/zopfli_t${test_num} test_input.txt 2>&1)
result=$?
log+="Command output: ${output}"$'\n'
if [ $result -eq 0 ] && [ -f "test_input.txt.gz" ] && [ -s "test_input.txt.gz" ]; then
    echo "Test ${test_num} passed"
    echo "${log}" > genifai_results/test${test_num}_success.log
else
    echo "Test ${test_num} failed"
    echo "Test ${test_num} failed" >&2
    echo "${log}" > genifai_results/test${test_num}_fail.log
    failed=1
fi
echo "Test ${test_num} ended"

############################################
# Test 24: Second compression with different options
############################################
test_num=24
echo "Test ${test_num} started"
log=""
rm -f test_input.txt.gz
output=$(LD_PRELOAD=libtracer.so TRACE_OUTPUT=$PWD/genifai_results/test${test_num}_trace.log ./${BINDIR}/zopfli_t${test_num} --i1 test_input.txt 2>&1)
result=$?
log+="Command output: ${output}"$'\n'
if [ $result -eq 0 ] && [ -f "test_input.txt.gz" ] && [ -s "test_input.txt.gz" ]; then
    echo "Test ${test_num} passed"
    echo "${log}" > genifai_results/test${test_num}_success.log
else
    echo "Test ${test_num} failed"
    echo "Test ${test_num} failed" >&2
    echo "${log}" > genifai_results/test${test_num}_fail.log
    failed=1
fi
echo "Test ${test_num} ended"

############################################
# Cleanup
############################################
rm -f test_input.txt test_input.txt.gz test_input.txt.zlib test_input.txt.deflate
rm -f empty_file.txt empty_file.txt.gz
rm -f special_chars.txt special_chars.txt.gz

exit $failed

