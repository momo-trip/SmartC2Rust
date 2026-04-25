
#!/bin/bash

# Reformed test cases

mkdir -p flow_results

failed=0

run_test() {
    local test_num=$1
    local binary="./test_markandsweep_t${test_num}"

    echo "Test ${test_num} started"

    if [ ! -f "$binary" ]; then
        echo "Test ${test_num} failed" >&2
        echo "Binary $binary not found" > "flow_results/test${test_num}_fail.log"
        failed=1
        echo "Test ${test_num} ended"
        return
    fi

    output=$(LD_PRELOAD=libtracer.so TRACE_OUTPUT=$PWD/flow_results/test${test_num}_trace.log "$binary" "$test_num" 2>&1)
    exit_code=$?

    if [ $exit_code -eq 0 ]; then
        echo "Test ${test_num} passed"
        echo "$output" > "flow_results/test${test_num}_success.log"
    else
        echo "Test ${test_num} failed" >&2
        echo "$output" > "flow_results/test${test_num}_fail.log"
        failed=1
    fi

    echo "Test ${test_num} ended"
}

for i in 1 2 3 4 5; do
    run_test "$i"
done

exit $failed

