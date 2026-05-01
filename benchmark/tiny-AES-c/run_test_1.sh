
#!/bin/bash

# Reformed test cases

failed=0

# Directories for test results
ACTUAL_DIR="actual"
EXPECTED_DIR="expected"
RESULTS_DIR="flow_results"

mkdir -p "${ACTUAL_DIR}"
mkdir -p "${RESULTS_DIR}"

# ---------------- Test 2: AES192 ----------------
test_num=2
echo "Test ${test_num} started"

output_file="${ACTUAL_DIR}/test${test_num}_output.log"
expected_file="${EXPECTED_DIR}/test${test_num}_output.log"
success_log="${RESULTS_DIR}/test${test_num}_success.log"
fail_log="${RESULTS_DIR}/test${test_num}_fail.log"

if [ ! -x ./test_t2.elf ]; then
    echo "Test ${test_num} failed" >&2
    echo "Binary test_t2.elf not found or not executable" > "${fail_log}"
    failed=1
    echo "Test ${test_num} ended"
else
    LD_PRELOAD=libtracer.so TRACE_OUTPUT=$PWD/flow_results/test${test_num}_trace.log ./test_t2.elf > "${output_file}" 2>&1

    test_failed=0
    line_num=0
    failure_details=""

    while IFS= read -r expected_line; do
        line_num=$((line_num + 1))

        # Skip empty lines
        [ -z "$expected_line" ] && continue

        actual_line=$(sed -n "${line_num}p" "${output_file}")

        if [[ ! "$actual_line" == "$expected_line"* ]]; then
            test_failed=1
            failure_details="${failure_details}Line ${line_num} does not match\nExpected: ${expected_line}\nActual: ${actual_line}\n\n"
        fi
    done < "${expected_file}"

    if [ $test_failed -eq 0 ]; then
        echo "AES test case ${test_num} with args 'aes192' passed successfully" > "${success_log}"
        echo "Test ${test_num} passed"
    else
        echo -e "AES test case ${test_num} with args 'aes192' failed:\n\n${failure_details}" > "${fail_log}"
        echo "Test ${test_num} failed" >&2
        failed=1
    fi

    echo "Test ${test_num} ended"
fi

exit $failed

