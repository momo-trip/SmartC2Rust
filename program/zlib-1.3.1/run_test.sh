
#!/bin/bash
# Reformed test cases for zlib-1.3.1 (testing minigzip)

set -u
failed=0

mkdir -p flow_results

# ---------------------------------------------------------------------------
# Test 1: Round-trip compression/decompression of a small text file (pipe mode)
# ---------------------------------------------------------------------------
echo "Test 1 started"
BIN=./minigzip_t1
log=flow_results/test1.tmp.log
: > "$log"
echo "hello world" > /tmp/t1.txt
LD_PRELOAD=libtracer.so TRACE_OUTPUT=$PWD/flow_results/test1_trace.log $BIN < /tmp/t1.txt > /tmp/t1.gz 2>>"$log"
LD_PRELOAD=libtracer.so TRACE_OUTPUT=$PWD/flow_results/test1_trace.log $BIN -d < /tmp/t1.gz > /tmp/t1.out 2>>"$log"
if cmp -s /tmp/t1.txt /tmp/t1.out; then
    echo "Test 1 passed"
    cat "$log" > flow_results/test1_success.log
else
    echo "test1 failed" >> "$log"
    echo "Test 1 failed"
    echo "Test 1 failed" >&2
    cat "$log" > flow_results/test1_fail.log
    failed=1
fi
rm -f "$log"
echo "Test 1 ended"

# ---------------------------------------------------------------------------
# Test 2: Round-trip compression/decompression of a larger text file (README)
# ---------------------------------------------------------------------------
echo "Test 2 started"
BIN=./minigzip_t2
log=flow_results/test2.tmp.log
: > "$log"
LD_PRELOAD=libtracer.so TRACE_OUTPUT=$PWD/flow_results/test2_trace.log $BIN < README > /tmp/t2.gz 2>>"$log"
LD_PRELOAD=libtracer.so TRACE_OUTPUT=$PWD/flow_results/test2_trace.log $BIN -d < /tmp/t2.gz > /tmp/t2.out 2>>"$log"
if cmp -s README /tmp/t2.out; then
    echo "Test 2 passed"
    cat "$log" > flow_results/test2_success.log
else
    echo "test2 failed" >> "$log"
    echo "Test 2 failed"
    echo "Test 2 failed" >&2
    cat "$log" > flow_results/test2_fail.log
    failed=1
fi
rm -f "$log"
echo "Test 2 ended"

# ---------------------------------------------------------------------------
# Test 3: Round-trip compression/decompression of binary data
# ---------------------------------------------------------------------------
echo "Test 3 started"
BIN=./minigzip_t3
log=flow_results/test3.tmp.log
: > "$log"
head -c 65536 /dev/urandom > /tmp/t3.bin
LD_PRELOAD=libtracer.so TRACE_OUTPUT=$PWD/flow_results/test3_trace.log $BIN < /tmp/t3.bin > /tmp/t3.gz 2>>"$log"
LD_PRELOAD=libtracer.so TRACE_OUTPUT=$PWD/flow_results/test3_trace.log $BIN -d < /tmp/t3.gz > /tmp/t3.out 2>>"$log"
if cmp -s /tmp/t3.bin /tmp/t3.out; then
    echo "Test 3 passed"
    cat "$log" > flow_results/test3_success.log
else
    echo "test3 failed" >> "$log"
    echo "Test 3 failed"
    echo "Test 3 failed" >&2
    cat "$log" > flow_results/test3_fail.log
    failed=1
fi
rm -f "$log"
echo "Test 3 ended"

# ---------------------------------------------------------------------------
# Test 4: Compression with maximum compression level (-9)
# ---------------------------------------------------------------------------
echo "Test 4 started"
BIN=./minigzip_t4
log=flow_results/test4.tmp.log
: > "$log"
LD_PRELOAD=libtracer.so TRACE_OUTPUT=$PWD/flow_results/test4_trace.log $BIN -9 < README > /tmp/t4.gz 2>>"$log"
LD_PRELOAD=libtracer.so TRACE_OUTPUT=$PWD/flow_results/test4_trace.log $BIN -d < /tmp/t4.gz > /tmp/t4.out 2>>"$log"
if cmp -s README /tmp/t4.out; then
    echo "Test 4 passed"
    cat "$log" > flow_results/test4_success.log
else
    echo "test4 failed" >> "$log"
    echo "Test 4 failed"
    echo "Test 4 failed" >&2
    cat "$log" > flow_results/test4_fail.log
    failed=1
fi
rm -f "$log"
echo "Test 4 ended"

# ---------------------------------------------------------------------------
# Test 5: Compression with fastest compression level (-1)
# ---------------------------------------------------------------------------
echo "Test 5 started"
BIN=./minigzip_t5
log=flow_results/test5.tmp.log
: > "$log"
LD_PRELOAD=libtracer.so TRACE_OUTPUT=$PWD/flow_results/test5_trace.log $BIN -1 < README > /tmp/t5.gz 2>>"$log"
LD_PRELOAD=libtracer.so TRACE_OUTPUT=$PWD/flow_results/test5_trace.log $BIN -d < /tmp/t5.gz > /tmp/t5.out 2>>"$log"
if cmp -s README /tmp/t5.out; then
    echo "Test 5 passed"
    cat "$log" > flow_results/test5_success.log
else
    echo "test5 failed" >> "$log"
    echo "Test 5 failed"
    echo "Test 5 failed" >&2
    cat "$log" > flow_results/test5_fail.log
    failed=1
fi
rm -f "$log"
echo "Test 5 ended"

# ---------------------------------------------------------------------------
# Test 6: File mode (minigzip file -> file.gz, then restore with -d)
# ---------------------------------------------------------------------------
echo "Test 6 started"
BIN=./minigzip_t6
log=flow_results/test6.tmp.log
: > "$log"
cp README /tmp/t6.txt
LD_PRELOAD=libtracer.so TRACE_OUTPUT=$PWD/flow_results/test6_trace.log $BIN /tmp/t6.txt 2>>"$log"
LD_PRELOAD=libtracer.so TRACE_OUTPUT=$PWD/flow_results/test6_trace.log $BIN -d /tmp/t6.txt.gz 2>>"$log"
if cmp -s README /tmp/t6.txt; then
    echo "Test 6 passed"
    cat "$log" > flow_results/test6_success.log
else
    echo "test6 failed" >> "$log"
    echo "Test 6 failed"
    echo "Test 6 failed" >&2
    cat "$log" > flow_results/test6_fail.log
    failed=1
fi
rm -f "$log"
echo "Test 6 ended"

# ---------------------------------------------------------------------------
# Test 7: Gzip compatibility (output can be decompressed by system gzip)
# ---------------------------------------------------------------------------
echo "Test 7 started"
BIN=./minigzip_t7
log=flow_results/test7.tmp.log
: > "$log"
echo "compat check" > /tmp/t7.txt
LD_PRELOAD=libtracer.so TRACE_OUTPUT=$PWD/flow_results/test7_trace.log $BIN < /tmp/t7.txt > /tmp/t7.gz 2>>"$log"
gzip -dc /tmp/t7.gz > /tmp/t7.out 2>>"$log"
if cmp -s /tmp/t7.txt /tmp/t7.out; then
    echo "Test 7 passed"
    cat "$log" > flow_results/test7_success.log
else
    echo "test7 failed" >> "$log"
    echo "Test 7 failed"
    echo "Test 7 failed" >&2
    cat "$log" > flow_results/test7_fail.log
    failed=1
fi
rm -f "$log"
echo "Test 7 ended"

# ---------------------------------------------------------------------------
# Test 8: Empty input file
# ---------------------------------------------------------------------------
echo "Test 8 started"
BIN=./minigzip_t8
log=flow_results/test8.tmp.log
: > "$log"
: > /tmp/t8.txt
LD_PRELOAD=libtracer.so TRACE_OUTPUT=$PWD/flow_results/test8_trace.log $BIN < /tmp/t8.txt > /tmp/t8.gz 2>>"$log"
LD_PRELOAD=libtracer.so TRACE_OUTPUT=$PWD/flow_results/test8_trace.log $BIN -d < /tmp/t8.gz > /tmp/t8.out 2>>"$log"
if cmp -s /tmp/t8.txt /tmp/t8.out; then
    echo "Test 8 passed"
    cat "$log" > flow_results/test8_success.log
else
    echo "test8 failed" >> "$log"
    echo "Test 8 failed"
    echo "Test 8 failed" >&2
    cat "$log" > flow_results/test8_fail.log
    failed=1
fi
rm -f "$log"
echo "Test 8 ended"

exit $failed

