
#!/bin/bash

# Reformed test cases

failed=0

mkdir -p flow_results

# Check that binaries were built
for i in 1 2 3 4 5; do
    if [ ! -f ./test_markandsweep_t${i} ]; then
        echo "Build failed: test_markandsweep_t${i} not found" >&2
        exit 1
    fi
done

# ---------------- Test 1 ----------------
test_num=1
echo "Test ${test_num} started"
output=$(LD_PRELOAD=libtracer.so TRACE_OUTPUT=$PWD/flow_results/test${test_num}_trace.log ./test_markandsweep_t${test_num} ${test_num} 2>&1)
expected_output=$(cat expected/test${test_num}.log)
if [ "$output" == "$expected_output" ]; then
    echo "Test ${test_num} passed"
    echo "$output" > "flow_results/test${test_num}_success.log"
else
    echo "Test ${test_num} failed" >&2
    echo "$output" > "flow_results/test${test_num}_fail.log"
    failed=1
fi
echo "Test ${test_num} ended"

# ---------------- Test 2 ----------------
test_num=2
echo "Test ${test_num} started"
output=$(LD_PRELOAD=libtracer.so TRACE_OUTPUT=$PWD/flow_results/test${test_num}_trace.log ./test_markandsweep_t${test_num} ${test_num} 2>&1)
expected_output=$(cat expected/test${test_num}.log)
if [ "$output" == "$expected_output" ]; then
    echo "Test ${test_num} passed"
    echo "$output" > "flow_results/test${test_num}_success.log"
else
    echo "Test ${test_num} failed" >&2
    echo "$output" > "flow_results/test${test_num}_fail.log"
    failed=1
fi
echo "Test ${test_num} ended"

# ---------------- Test 3 ----------------
test_num=3
echo "Test ${test_num} started"
output=$(LD_PRELOAD=libtracer.so TRACE_OUTPUT=$PWD/flow_results/test${test_num}_trace.log ./test_markandsweep_t${test_num} ${test_num} 2>&1)
expected_output=$(cat expected/test${test_num}.log)
if [ "$output" == "$expected_output" ]; then
    echo "Test ${test_num} passed"
    echo "$output" > "flow_results/test${test_num}_success.log"
else
    echo "Test ${test_num} failed" >&2
    echo "$output" > "flow_results/test${test_num}_fail.log"
    failed=1
fi
echo "Test ${test_num} ended"

# ---------------- Test 4 ----------------
test_num=4
echo "Test ${test_num} started"
output=$(LD_PRELOAD=libtracer.so TRACE_OUTPUT=$PWD/flow_results/test${test_num}_trace.log ./test_markandsweep_t${test_num} ${test_num} 2>&1)
expected_output=$(cat expected/test${test_num}.log)
if [ "$output" == "$expected_output" ]; then
    echo "Test ${test_num} passed"
    echo "$output" > "flow_results/test${test_num}_success.log"
else
    echo "Test ${test_num} failed" >&2
    echo "$output" > "flow_results/test${test_num}_fail.log"
    failed=1
fi
echo "Test ${test_num} ended"

# ---------------- Test 5 ----------------
test_num=5
echo "Test ${test_num} started"
output=$(LD_PRELOAD=libtracer.so TRACE_OUTPUT=$PWD/flow_results/test${test_num}_trace.log ./test_markandsweep_t${test_num} ${test_num} 2>&1)
expected_output=$(cat expected/test${test_num}.log)
if [ "$output" == "$expected_output" ]; then
    echo "Test ${test_num} passed"
    echo "$output" > "flow_results/test${test_num}_success.log"
else
    echo "Test ${test_num} failed" >&2
    echo "$output" > "flow_results/test${test_num}_fail.log"
    failed=1
fi
echo "Test ${test_num} ended"

exit $failed

