
#!/bin/bash

# Reformed test cases

failed=0
mkdir -p flow_results

EXPECTED_DIR="expected"

run_test_case() {
    local test_num=$1
    local binary="./test_t${test_num}"
    local expected_file="${EXPECTED_DIR}/expected_values_${test_num}.txt"
    local success_log="flow_results/test${test_num}_success.log"
    local fail_log="flow_results/test${test_num}_fail.log"
    local tmp_log="flow_results/test${test_num}_tmp.log"

    echo "Test ${test_num} started"

    if [ ! -x "${binary}" ]; then
        echo "Binary ${binary} not found or not executable" > "${fail_log}"
        echo "Test ${test_num} failed" >&2
        echo "Test ${test_num} failed"
        failed=1
        echo "Test ${test_num} ended"
        return
    fi

    LD_PRELOAD=libtracer.so TRACE_OUTPUT=$PWD/flow_results/test${test_num}_trace.log ${binary} ${test_num} > "${tmp_log}" 2>&1
    local exit_code=$?

    local actual_output
    actual_output=$(cat "${tmp_log}")

    if [ ! -f "${expected_file}" ]; then
        {
            echo "Expected values file not found: ${expected_file}"
            echo "Actual output:"
            echo "${actual_output}"
        } > "${fail_log}"
        rm -f "${tmp_log}"
        echo "Test ${test_num} failed" >&2
        echo "Test ${test_num} failed"
        failed=1
        echo "Test ${test_num} ended"
        return
    fi

    local expected_output
    expected_output=$(cat "${expected_file}")

    if [ ${exit_code} -eq 0 ] && [ "${actual_output}" = "${expected_output}" ]; then
        {
            echo "Test ${test_num} passed"
            echo "=== Output ==="
            echo "${actual_output}"
        } > "${success_log}"
        rm -f "${tmp_log}"
        echo "Test ${test_num} passed"
    else
        {
            echo "Test ${test_num} failed"
            echo "Exit code: ${exit_code}"
            echo "=== Expected ==="
            echo "${expected_output}"
            echo "=== Actual ==="
            echo "${actual_output}"
            echo "=== Diff ==="
            diff <(echo "${expected_output}") <(echo "${actual_output}")
        } > "${fail_log}"
        rm -f "${tmp_log}"
        echo "Test ${test_num} failed" >&2
        echo "Test ${test_num} failed"
        failed=1
    fi

    echo "Test ${test_num} ended"
}

for i in $(seq 1 17); do
    run_test_case $i
done

exit $failed

