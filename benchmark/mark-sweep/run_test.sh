
#!/bin/bash

# Reformed test cases

failed=0
mkdir -p flow_results

run_case() {
    local test_num=$1
    local banner=$2
    echo "Test ${test_num} started"
    local binary=./test_markandsweep_t${test_num}
    local success_log=flow_results/test${test_num}_success.log
    local fail_log=flow_results/test${test_num}_fail.log
    local trace_log=$PWD/flow_results/test${test_num}_trace.log
    local output_log=flow_results/test${test_num}_output.log

    if [ ! -x "$binary" ]; then
        echo "Binary $binary not found" > "$fail_log"
        echo "Test ${test_num} failed"
        echo "Test ${test_num} failed" >&2
        failed=1
        echo "Test ${test_num} ended"
        return
    fi

    LD_PRELOAD=libtracer.so TRACE_OUTPUT=$trace_log $binary > "$output_log" 2>&1
    rc=$?

    local pass=1
    if [ $rc -ne 0 ]; then
        pass=0
    fi
    if [ ! -s "$output_log" ]; then
        pass=0
    fi
    if ! grep -qF "$banner" "$output_log"; then
        pass=0
    fi

    if [ $pass -eq 1 ]; then
        cp "$output_log" "$success_log"
        rm -f "$fail_log"
        echo "Test ${test_num} passed"
    else
        cp "$output_log" "$fail_log"
        echo "Test ${test_num} failed"
        echo "Test ${test_num} failed" >&2
        failed=1
    fi
    rm -f "$output_log"
    echo "Test ${test_num} ended"
}

run_case 1 "Test 1: Objects on stack are preserved."
run_case 2 "Test 2: Unreached objects are collected."
run_case 3 "Test 3: Reach nested objects."
run_case 4 "Test 4: Handle cycles."
run_case 5 "Performance Test."

exit $failed

