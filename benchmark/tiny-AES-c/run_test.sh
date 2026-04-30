
#!/bin/bash

# Reformed test cases

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$SCRIPT_DIR"

failed=0

mkdir -p flow_results

ACTUAL_DIR="actual"
EXPECTED_DIR="expected"
mkdir -p "${ACTUAL_DIR}"

# -------------------- Test 1: Default AES128 --------------------
test_num=1
echo "Test ${test_num} started"

output_file="${ACTUAL_DIR}/test${test_num}_output.log"
expected_file="${EXPECTED_DIR}/test${test_num}_output.log"
success_log="flow_results/test${test_num}_success.log"
fail_log="flow_results/test${test_num}_fail.log"

test_failed=0
failure_details=""

LD_PRELOAD=libtracer.so TRACE_OUTPUT=$PWD/flow_results/test${test_num}_trace.log ./test_t1.elf > "${output_file}" 2>&1

if [ ! -f "${expected_file}" ]; then
    echo "Expected file not found: ${expected_file}" >&2
    failure_details="Expected file not found: ${expected_file}\n"
    test_failed=1
fi

line_num=0
while IFS= read -r expected_line; do
    line_num=$((line_num + 1))
    [ -z "$expected_line" ] && continue
    actual_line=$(sed -n "${line_num}p" "${output_file}")
    if [[ ! "$actual_line" == "$expected_line"* ]]; then
        test_failed=1
        echo "Line ${line_num} does not match"
        echo "Expected: $expected_line"
        echo "Actual: $actual_line"
        failure_details="${failure_details}Line ${line_num} does not match\nExpected: ${expected_line}\nActual: ${actual_line}\n\n"
    fi
done < "${expected_file}"

if [ $test_failed -eq 0 ]; then
    echo "Test ${test_num} passed"
    echo "AES test case ${test_num} passed successfully" > "${success_log}"
else
    echo "Test ${test_num} failed" >&2
    echo -e "AES test case ${test_num} failed:\n\n${failure_details}" > "${fail_log}"
    failed=1
fi

echo "Test ${test_num} ended"

exit $failed

