
#!/bin/bash

# Reformed test cases

cd "$(dirname "$0")"

failed=0

# Directories for test results
ACTUAL_DIR="actual"
EXPECTED_DIR="expected"
mkdir -p flow_results
mkdir -p "${ACTUAL_DIR}"

# ============ Test 1: Default AES (AES128) ============
echo "Test 1 started"

output_file="${ACTUAL_DIR}/test1_output.log"
expected_file="${EXPECTED_DIR}/test1_output.log"
log_file="flow_results/test1_output.log"

LD_PRELOAD=libtracer.so TRACE_OUTPUT=$PWD/flow_results/test1_trace.log ./test_aes_t1 > "${output_file}" 2>&1
test1_exit=$?

test1_failed=0

if [ $test1_exit -ne 0 ]; then
    test1_failed=1
    echo "Test binary exited with code ${test1_exit}" > "${log_file}"
fi

if [ -f "${expected_file}" ]; then
    line_num=0
    failure_details=""
    while IFS= read -r expected_line; do
        line_num=$((line_num + 1))
        [ -z "$expected_line" ] && continue
        actual_line=$(sed -n "${line_num}p" "${output_file}")
        if [[ ! "$actual_line" == "$expected_line"* ]]; then
            test1_failed=1
            failure_details="${failure_details}Line ${line_num} does not match\nExpected: ${expected_line}\nActual: ${actual_line}\n\n"
        fi
    done < "${expected_file}"
fi

if [ $test1_failed -eq 0 ]; then
    echo "Test 1 passed"
    echo "AES test case 1 (default AES128) passed successfully" > flow_results/test1_success.log
    cat "${output_file}" >> flow_results/test1_success.log
else
    echo "Test 1 failed"
    echo "Test 1 failed" >&2
    failed=1
    echo -e "AES test case 1 (default AES128) failed:\n\n${failure_details}" > flow_results/test1_fail.log
    cat "${output_file}" >> flow_results/test1_fail.log
fi

echo "Test 1 ended"

exit $failed

