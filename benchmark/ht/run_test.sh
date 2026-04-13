
#!/bin/bash

# Reformed test cases
failed=0

mkdir -p flow_results
rm -rf samples/output
mkdir -p samples/output

# ============================================================
# Test 1: lsearch
# ============================================================
echo "Test 1 started"
test1_log=""
test1_pass=1

test1_output=$(LD_PRELOAD=libtracer.so TRACE_OUTPUT=$PWD/flow_results/test1_trace.log ./lsearch_t1 2>&1)
if [ $? -ne 0 ]; then
    test1_pass=0
    test1_log+="lsearch_t1 execution failed\n"
fi

echo "$test1_output" > samples/output/lsearch.txt

if [ -f "expected/lsearch.txt" ]; then
    diff_result=$(diff expected/lsearch.txt samples/output/lsearch.txt 2>&1)
    if [ $? -ne 0 ]; then
        test1_pass=0
        test1_log+="lsearch.txt differs from expected:\n$diff_result\n"
    fi
else
    test1_pass=0
    test1_log+="Expected file expected/lsearch.txt not found\n"
fi

if [ $test1_pass -eq 1 ]; then
    echo "Test 1 passed"
    echo -e "Test 1 passed\nOutput:\n$test1_output" > flow_results/test1_success.log
else
    echo "Test 1 failed"
    echo "Test 1 failed" >&2
    echo -e "Test 1 failed\n$test1_log" > flow_results/test1_fail.log
    failed=1
fi
echo "Test 1 ended"

# ============================================================
# Test 2: bsearch
# ============================================================
echo "Test 2 started"
test2_log=""
test2_pass=1

test2_output=$(LD_PRELOAD=libtracer.so TRACE_OUTPUT=$PWD/flow_results/test2_trace.log ./bsearch_t2 2>&1)
if [ $? -ne 0 ]; then
    test2_pass=0
    test2_log+="bsearch_t2 execution failed\n"
fi

echo "$test2_output" > samples/output/bsearch.txt

if [ -f "expected/bsearch.txt" ]; then
    diff_result=$(diff expected/bsearch.txt samples/output/bsearch.txt 2>&1)
    if [ $? -ne 0 ]; then
        test2_pass=0
        test2_log+="bsearch.txt differs from expected:\n$diff_result\n"
    fi
else
    test2_pass=0
    test2_log+="Expected file expected/bsearch.txt not found\n"
fi

if [ $test2_pass -eq 1 ]; then
    echo "Test 2 passed"
    echo -e "Test 2 passed\nOutput:\n$test2_output" > flow_results/test2_success.log
else
    echo "Test 2 failed"
    echo "Test 2 failed" >&2
    echo -e "Test 2 failed\n$test2_log" > flow_results/test2_fail.log
    failed=1
fi
echo "Test 2 ended"

# ============================================================
# Test 3: demo
# ============================================================
echo "Test 3 started"
test3_log=""
test3_pass=1

test3_output=$(echo 'foo bar the bar bar bar the' | LD_PRELOAD=libtracer.so TRACE_OUTPUT=$PWD/flow_results/test3_trace.log ./demo_t3 2>&1)
if [ $? -ne 0 ]; then
    test3_pass=0
    test3_log+="demo_t3 execution failed\n"
fi

echo "$test3_output" > samples/output/demo.txt

if [ -f "expected/demo.txt" ]; then
    diff_result=$(diff expected/demo.txt samples/output/demo.txt 2>&1)
    if [ $? -ne 0 ]; then
        test3_pass=0
        test3_log+="demo.txt differs from expected:\n$diff_result\n"
    fi
else
    test3_pass=0
    test3_log+="Expected file expected/demo.txt not found\n"
fi

if [ $test3_pass -eq 1 ]; then
    echo "Test 3 passed"
    echo -e "Test 3 passed\nOutput:\n$test3_output" > flow_results/test3_success.log
else
    echo "Test 3 failed"
    echo "Test 3 failed" >&2
    echo -e "Test 3 failed\n$test3_log" > flow_results/test3_fail.log
    failed=1
fi
echo "Test 3 ended"

# ============================================================
# Test 4: dump
# ============================================================
echo "Test 4 started"
test4_log=""
test4_pass=1

test4_output=$(LD_PRELOAD=libtracer.so TRACE_OUTPUT=$PWD/flow_results/test4_trace.log ./dump_t4 2>&1)
if [ $? -ne 0 ]; then
    test4_pass=0
    test4_log+="dump_t4 execution failed\n"
fi

echo "$test4_output" > samples/output/dump.txt

if [ -f "expected/dump.txt" ]; then
    diff_result=$(diff expected/dump.txt samples/output/dump.txt 2>&1)
    if [ $? -ne 0 ]; then
        test4_pass=0
        test4_log+="dump.txt differs from expected:\n$diff_result\n"
    fi
else
    test4_pass=0
    test4_log+="Expected file expected/dump.txt not found\n"
fi

if [ $test4_pass -eq 1 ]; then
    echo "Test 4 passed"
    echo -e "Test 4 passed\nOutput:\n$test4_output" > flow_results/test4_success.log
else
    echo "Test 4 failed"
    echo "Test 4 failed" >&2
    echo -e "Test 4 failed\n$test4_log" > flow_results/test4_fail.log
    failed=1
fi
echo "Test 4 ended"

# ============================================================
# Generate similar.txt for tests 5 and 6
# ============================================================
python3 samples/gensimilar.py 466550 > samples/similar.txt

# ============================================================
# Test 5: stats with words.txt
# ============================================================
echo "Test 5 started"
test5_log=""
test5_pass=1

test5_output=$(LD_PRELOAD=libtracer.so TRACE_OUTPUT=$PWD/flow_results/test5_trace.log ./stats_t5 < samples/words.txt 2>&1)
if [ $? -ne 0 ]; then
    test5_pass=0
    test5_log+="stats_t5 execution failed\n"
fi

echo "$test5_output" > samples/output/stats-words.txt

if [ -f "expected/stats-words.txt" ]; then
    diff_result=$(diff expected/stats-words.txt samples/output/stats-words.txt 2>&1)
    if [ $? -ne 0 ]; then
        test5_pass=0
        test5_log+="stats-words.txt differs from expected:\n$diff_result\n"
    fi
else
    test5_pass=0
    test5_log+="Expected file expected/stats-words.txt not found\n"
fi

if [ $test5_pass -eq 1 ]; then
    echo "Test 5 passed"
    echo -e "Test 5 passed\nOutput:\n$test5_output" > flow_results/test5_success.log
else
    echo "Test 5 failed"
    echo "Test 5 failed" >&2
    echo -e "Test 5 failed\n$test5_log" > flow_results/test5_fail.log
    failed=1
fi
echo "Test 5 ended"

# ============================================================
# Test 6: stats with similar.txt
# ============================================================
echo "Test 6 started"
test6_log=""
test6_pass=1

test6_output=$(LD_PRELOAD=libtracer.so TRACE_OUTPUT=$PWD/flow_results/test6_trace.log ./stats_t6 < samples/similar.txt 2>&1)
if [ $? -ne 0 ]; then
    test6_pass=0
    test6_log+="stats_t6 execution failed\n"
fi

echo "$test6_output" > samples/output/stats-similar.txt

if [ -f "expected/stats-similar.txt" ]; then
    diff_result=$(diff expected/stats-similar.txt samples/output/stats-similar.txt 2>&1)
    if [ $? -ne 0 ]; then
        test6_pass=0
        test6_log+="stats-similar.txt differs from expected:\n$diff_result\n"
    fi
else
    test6_pass=0
    test6_log+="Expected file expected/stats-similar.txt not found\n"
fi

if [ $test6_pass -eq 1 ]; then
    echo "Test 6 passed"
    echo -e "Test 6 passed\nOutput:\n$test6_output" > flow_results/test6_success.log
else
    echo "Test 6 failed"
    echo "Test 6 failed" >&2
    echo -e "Test 6 failed\n$test6_log" > flow_results/test6_fail.log
    failed=1
fi
echo "Test 6 ended"

exit $failed

