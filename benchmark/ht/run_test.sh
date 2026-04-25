
#!/bin/bash

# Reformed test cases

failed=0

mkdir -p flow_results
mkdir -p samples/output

# Test 1: lsearch
test_num=1
echo "Test ${test_num} started"
LOG_FILE="flow_results/test${test_num}_tmp.log"
OUT_FILE="samples/output/lsearch.txt"
EXPECTED="expected/lsearch.txt"
LD_PRELOAD=libtracer.so TRACE_OUTPUT=$PWD/flow_results/test${test_num}_trace.log ./lsearch_t1 > "$OUT_FILE" 2> "$LOG_FILE"
rc=$?
if [ $rc -eq 0 ] && diff "$EXPECTED" "$OUT_FILE" >> "$LOG_FILE" 2>&1; then
    echo "Test ${test_num} passed"
    mv "$LOG_FILE" "flow_results/test${test_num}_success.log"
else
    echo "Test ${test_num} failed" >&2
    mv "$LOG_FILE" "flow_results/test${test_num}_fail.log"
    failed=1
fi
echo "Test ${test_num} ended"

# Test 2: bsearch
test_num=2
echo "Test ${test_num} started"
LOG_FILE="flow_results/test${test_num}_tmp.log"
OUT_FILE="samples/output/bsearch.txt"
EXPECTED="expected/bsearch.txt"
LD_PRELOAD=libtracer.so TRACE_OUTPUT=$PWD/flow_results/test${test_num}_trace.log ./bsearch_t2 > "$OUT_FILE" 2> "$LOG_FILE"
rc=$?
if [ $rc -eq 0 ] && diff "$EXPECTED" "$OUT_FILE" >> "$LOG_FILE" 2>&1; then
    echo "Test ${test_num} passed"
    mv "$LOG_FILE" "flow_results/test${test_num}_success.log"
else
    echo "Test ${test_num} failed" >&2
    mv "$LOG_FILE" "flow_results/test${test_num}_fail.log"
    failed=1
fi
echo "Test ${test_num} ended"

# Test 3: demo
test_num=3
echo "Test ${test_num} started"
LOG_FILE="flow_results/test${test_num}_tmp.log"
OUT_FILE="samples/output/demo.txt"
EXPECTED="expected/demo.txt"
echo 'foo bar the bar bar bar the' | LD_PRELOAD=libtracer.so TRACE_OUTPUT=$PWD/flow_results/test${test_num}_trace.log ./demo_t3 > "$OUT_FILE" 2> "$LOG_FILE"
rc=$?
if [ $rc -eq 0 ] && diff "$EXPECTED" "$OUT_FILE" >> "$LOG_FILE" 2>&1; then
    echo "Test ${test_num} passed"
    mv "$LOG_FILE" "flow_results/test${test_num}_success.log"
else
    echo "Test ${test_num} failed" >&2
    mv "$LOG_FILE" "flow_results/test${test_num}_fail.log"
    failed=1
fi
echo "Test ${test_num} ended"

# Test 4: dump
test_num=4
echo "Test ${test_num} started"
LOG_FILE="flow_results/test${test_num}_tmp.log"
OUT_FILE="samples/output/dump.txt"
EXPECTED="expected/dump.txt"
LD_PRELOAD=libtracer.so TRACE_OUTPUT=$PWD/flow_results/test${test_num}_trace.log ./dump_t4 > "$OUT_FILE" 2> "$LOG_FILE"
rc=$?
if [ $rc -eq 0 ] && diff "$EXPECTED" "$OUT_FILE" >> "$LOG_FILE" 2>&1; then
    echo "Test ${test_num} passed"
    mv "$LOG_FILE" "flow_results/test${test_num}_success.log"
else
    echo "Test ${test_num} failed" >&2
    mv "$LOG_FILE" "flow_results/test${test_num}_fail.log"
    failed=1
fi
echo "Test ${test_num} ended"

# Generate similar.txt for Test 6
if [ ! -f samples/similar.txt ] || [ $(wc -l < samples/similar.txt) -lt 466550 ]; then
    python3 samples/gensimilar.py 466550 > samples/similar.txt
fi

# Test 5: stats with words.txt
test_num=5
echo "Test ${test_num} started"
LOG_FILE="flow_results/test${test_num}_tmp.log"
OUT_FILE="samples/output/stats-words.txt"
EXPECTED="expected/stats-words.txt"
LD_PRELOAD=libtracer.so TRACE_OUTPUT=$PWD/flow_results/test${test_num}_trace.log ./stats_t5 < samples/words.txt > "$OUT_FILE" 2> "$LOG_FILE"
rc=$?
if [ $rc -eq 0 ] && diff "$EXPECTED" "$OUT_FILE" >> "$LOG_FILE" 2>&1; then
    echo "Test ${test_num} passed"
    mv "$LOG_FILE" "flow_results/test${test_num}_success.log"
else
    echo "Test ${test_num} failed" >&2
    mv "$LOG_FILE" "flow_results/test${test_num}_fail.log"
    failed=1
fi
echo "Test ${test_num} ended"

# Test 6: stats with similar.txt
test_num=6
echo "Test ${test_num} started"
LOG_FILE="flow_results/test${test_num}_tmp.log"
OUT_FILE="samples/output/stats-similar.txt"
EXPECTED="expected/stats-similar.txt"
LD_PRELOAD=libtracer.so TRACE_OUTPUT=$PWD/flow_results/test${test_num}_trace.log ./stats_t6 < samples/similar.txt > "$OUT_FILE" 2> "$LOG_FILE"
rc=$?
if [ $rc -eq 0 ] && diff "$EXPECTED" "$OUT_FILE" >> "$LOG_FILE" 2>&1; then
    echo "Test ${test_num} passed"
    mv "$LOG_FILE" "flow_results/test${test_num}_success.log"
else
    echo "Test ${test_num} failed" >&2
    mv "$LOG_FILE" "flow_results/test${test_num}_fail.log"
    failed=1
fi
echo "Test ${test_num} ended"

exit $failed

