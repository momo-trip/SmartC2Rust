
#!/bin/bash

# Reformed test cases

failed=0

rm -rf samples/output
mkdir -p samples/output
mkdir -p flow_results

# Test 1: lsearch
test_num=1
echo "Test ${test_num} started"
log_file="flow_results/test${test_num}_tmp.log"
: > "$log_file"
output_file="samples/output/lsearch.txt"
expected_file="expected/lsearch.txt"
echo "Running: ./lsearch_t1 >$output_file" >> "$log_file"
LD_PRELOAD=libtracer.so TRACE_OUTPUT=$PWD/flow_results/test${test_num}_trace.log ./lsearch_t1 >"$output_file" 2>> "$log_file"
rc=$?
test_ok=1
if [ $rc -ne 0 ]; then
    echo "command failed with exit $rc" >> "$log_file"
    test_ok=0
elif [ -f "$expected_file" ]; then
    if diff "$expected_file" "$output_file" >> "$log_file" 2>&1; then
        echo "$output_file matches expected" >> "$log_file"
    else
        echo "$output_file differs from expected" >> "$log_file"
        test_ok=0
    fi
else
    echo "Expected file $expected_file not found" >> "$log_file"
    test_ok=0
fi
if [ $test_ok -eq 1 ]; then
    mv "$log_file" "flow_results/test${test_num}_success.log"
    echo "Test ${test_num} passed"
else
    mv "$log_file" "flow_results/test${test_num}_fail.log"
    echo "Test ${test_num} failed" >&2
    failed=1
fi
echo "Test ${test_num} ended"

# Test 2: bsearch
test_num=2
echo "Test ${test_num} started"
log_file="flow_results/test${test_num}_tmp.log"
: > "$log_file"
output_file="samples/output/bsearch.txt"
expected_file="expected/bsearch.txt"
echo "Running: ./bsearch_t2 >$output_file" >> "$log_file"
LD_PRELOAD=libtracer.so TRACE_OUTPUT=$PWD/flow_results/test${test_num}_trace.log ./bsearch_t2 >"$output_file" 2>> "$log_file"
rc=$?
test_ok=1
if [ $rc -ne 0 ]; then
    echo "command failed with exit $rc" >> "$log_file"
    test_ok=0
elif [ -f "$expected_file" ]; then
    if diff "$expected_file" "$output_file" >> "$log_file" 2>&1; then
        echo "$output_file matches expected" >> "$log_file"
    else
        echo "$output_file differs from expected" >> "$log_file"
        test_ok=0
    fi
else
    echo "Expected file $expected_file not found" >> "$log_file"
    test_ok=0
fi
if [ $test_ok -eq 1 ]; then
    mv "$log_file" "flow_results/test${test_num}_success.log"
    echo "Test ${test_num} passed"
else
    mv "$log_file" "flow_results/test${test_num}_fail.log"
    echo "Test ${test_num} failed" >&2
    failed=1
fi
echo "Test ${test_num} ended"

# Test 3: demo
test_num=3
echo "Test ${test_num} started"
log_file="flow_results/test${test_num}_tmp.log"
: > "$log_file"
output_file="samples/output/demo.txt"
expected_file="expected/demo.txt"
echo "Running: echo 'foo bar the bar bar bar the' | ./demo_t3 >$output_file" >> "$log_file"
echo 'foo bar the bar bar bar the' | LD_PRELOAD=libtracer.so TRACE_OUTPUT=$PWD/flow_results/test${test_num}_trace.log ./demo_t3 >"$output_file" 2>> "$log_file"
rc=$?
test_ok=1
if [ $rc -ne 0 ]; then
    echo "command failed with exit $rc" >> "$log_file"
    test_ok=0
elif [ -f "$expected_file" ]; then
    if diff "$expected_file" "$output_file" >> "$log_file" 2>&1; then
        echo "$output_file matches expected" >> "$log_file"
    else
        echo "$output_file differs from expected" >> "$log_file"
        test_ok=0
    fi
else
    echo "Expected file $expected_file not found" >> "$log_file"
    test_ok=0
fi
if [ $test_ok -eq 1 ]; then
    mv "$log_file" "flow_results/test${test_num}_success.log"
    echo "Test ${test_num} passed"
else
    mv "$log_file" "flow_results/test${test_num}_fail.log"
    echo "Test ${test_num} failed" >&2
    failed=1
fi
echo "Test ${test_num} ended"

# Test 4: dump
test_num=4
echo "Test ${test_num} started"
log_file="flow_results/test${test_num}_tmp.log"
: > "$log_file"
output_file="samples/output/dump.txt"
expected_file="expected/dump.txt"
echo "Running: ./dump_t4 >$output_file" >> "$log_file"
LD_PRELOAD=libtracer.so TRACE_OUTPUT=$PWD/flow_results/test${test_num}_trace.log ./dump_t4 >"$output_file" 2>> "$log_file"
rc=$?
test_ok=1
if [ $rc -ne 0 ]; then
    echo "command failed with exit $rc" >> "$log_file"
    test_ok=0
elif [ -f "$expected_file" ]; then
    if diff "$expected_file" "$output_file" >> "$log_file" 2>&1; then
        echo "$output_file matches expected" >> "$log_file"
    else
        echo "$output_file differs from expected" >> "$log_file"
        test_ok=0
    fi
else
    echo "Expected file $expected_file not found" >> "$log_file"
    test_ok=0
fi
if [ $test_ok -eq 1 ]; then
    mv "$log_file" "flow_results/test${test_num}_success.log"
    echo "Test ${test_num} passed"
else
    mv "$log_file" "flow_results/test${test_num}_fail.log"
    echo "Test ${test_num} failed" >&2
    failed=1
fi
echo "Test ${test_num} ended"

# Test 5: stats with words.txt
test_num=5
echo "Test ${test_num} started"
log_file="flow_results/test${test_num}_tmp.log"
: > "$log_file"
output_file="samples/output/stats-words.txt"
expected_file="expected/stats-words.txt"
echo "Running: ./stats_t5 <samples/words.txt >$output_file" >> "$log_file"
LD_PRELOAD=libtracer.so TRACE_OUTPUT=$PWD/flow_results/test${test_num}_trace.log ./stats_t5 <samples/words.txt >"$output_file" 2>> "$log_file"
rc=$?
test_ok=1
if [ $rc -ne 0 ]; then
    echo "command failed with exit $rc" >> "$log_file"
    test_ok=0
elif [ -f "$expected_file" ]; then
    if diff "$expected_file" "$output_file" >> "$log_file" 2>&1; then
        echo "$output_file matches expected" >> "$log_file"
    else
        echo "$output_file differs from expected" >> "$log_file"
        test_ok=0
    fi
else
    echo "Expected file $expected_file not found" >> "$log_file"
    test_ok=0
fi
if [ $test_ok -eq 1 ]; then
    mv "$log_file" "flow_results/test${test_num}_success.log"
    echo "Test ${test_num} passed"
else
    mv "$log_file" "flow_results/test${test_num}_fail.log"
    echo "Test ${test_num} failed" >&2
    failed=1
fi
echo "Test ${test_num} ended"

# Test 6: stats with similar.txt
test_num=6
echo "Test ${test_num} started"
log_file="flow_results/test${test_num}_tmp.log"
: > "$log_file"
echo "Generating samples/similar.txt" >> "$log_file"
python3 samples/gensimilar.py 466550 >samples/similar.txt 2>> "$log_file"
gen_rc=$?
output_file="samples/output/stats-similar.txt"
expected_file="expected/stats-similar.txt"
test_ok=1
if [ $gen_rc -ne 0 ]; then
    echo "gensimilar.py failed with exit $gen_rc" >> "$log_file"
    test_ok=0
else
    echo "Running: ./stats_t6 <samples/similar.txt >$output_file" >> "$log_file"
    LD_PRELOAD=libtracer.so TRACE_OUTPUT=$PWD/flow_results/test${test_num}_trace.log ./stats_t6 <samples/similar.txt >"$output_file" 2>> "$log_file"
    rc=$?
    if [ $rc -ne 0 ]; then
        echo "command failed with exit $rc" >> "$log_file"
        test_ok=0
    elif [ -f "$expected_file" ]; then
        if diff "$expected_file" "$output_file" >> "$log_file" 2>&1; then
            echo "$output_file matches expected" >> "$log_file"
        else
            echo "$output_file differs from expected" >> "$log_file"
            test_ok=0
        fi
    else
        echo "Expected file $expected_file not found" >> "$log_file"
        test_ok=0
    fi
fi
if [ $test_ok -eq 1 ]; then
    mv "$log_file" "flow_results/test${test_num}_success.log"
    echo "Test ${test_num} passed"
else
    mv "$log_file" "flow_results/test${test_num}_fail.log"
    echo "Test ${test_num} failed" >&2
    failed=1
fi
echo "Test ${test_num} ended"

exit $failed

