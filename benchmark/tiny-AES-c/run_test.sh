
#!/bin/bash

# Reformed test cases

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$SCRIPT_DIR"

mkdir -p flow_results

failed=0

############################################
# Test 1: AES128 (default)
############################################
test_num=1
echo "Test ${test_num} started"
log_file="flow_results/test${test_num}_run.log"
: > "${log_file}"

LD_PRELOAD=libtracer.so TRACE_OUTPUT=$PWD/flow_results/test${test_num}_trace.log ./test_aes_t1 > "${log_file}" 2>&1
rc=$?

t1_ok=1
if [ $rc -ne 0 ]; then
    t1_ok=0
fi

# Verify expected output strings for AES128
if ! grep -q "Testing AES128" "${log_file}"; then t1_ok=0; fi
if ! grep -q "CBC encrypt: SUCCESS!" "${log_file}"; then t1_ok=0; fi
if ! grep -q "CBC decrypt: SUCCESS!" "${log_file}"; then t1_ok=0; fi
if ! grep -q "CTR encrypt: SUCCESS!" "${log_file}"; then t1_ok=0; fi
if ! grep -q "CTR decrypt: SUCCESS!" "${log_file}"; then t1_ok=0; fi
if ! grep -q "ECB encrypt: SUCCESS!" "${log_file}"; then t1_ok=0; fi
if ! grep -q "ECB decrypt: SUCCESS!" "${log_file}"; then t1_ok=0; fi
if ! grep -q "ECB encrypt verbose:" "${log_file}"; then t1_ok=0; fi

if [ $t1_ok -eq 1 ]; then
    cp "${log_file}" "flow_results/test${test_num}_success.log"
    echo "Test ${test_num} passed"
else
    cp "${log_file}" "flow_results/test${test_num}_fail.log"
    echo "Test ${test_num} failed" >&2
    failed=1
fi
echo "Test ${test_num} ended"

############################################
# Test 2: AES192
############################################
test_num=2
echo "Test ${test_num} started"
log_file="flow_results/test${test_num}_run.log"
: > "${log_file}"

LD_PRELOAD=libtracer.so TRACE_OUTPUT=$PWD/flow_results/test${test_num}_trace.log ./test_aes_t2 > "${log_file}" 2>&1
rc=$?

t2_ok=1
if [ $rc -ne 0 ]; then
    t2_ok=0
fi

if ! grep -q "Testing AES192" "${log_file}"; then t2_ok=0; fi
if ! grep -q "CBC encrypt: SUCCESS!" "${log_file}"; then t2_ok=0; fi
if ! grep -q "CBC decrypt: SUCCESS!" "${log_file}"; then t2_ok=0; fi
if ! grep -q "CTR encrypt: SUCCESS!" "${log_file}"; then t2_ok=0; fi
if ! grep -q "CTR decrypt: SUCCESS!" "${log_file}"; then t2_ok=0; fi
if ! grep -q "ECB encrypt: SUCCESS!" "${log_file}"; then t2_ok=0; fi
if ! grep -q "ECB decrypt: SUCCESS!" "${log_file}"; then t2_ok=0; fi
if ! grep -q "ECB encrypt verbose:" "${log_file}"; then t2_ok=0; fi

if [ $t2_ok -eq 1 ]; then
    cp "${log_file}" "flow_results/test${test_num}_success.log"
    echo "Test ${test_num} passed"
else
    cp "${log_file}" "flow_results/test${test_num}_fail.log"
    echo "Test ${test_num} failed" >&2
    failed=1
fi
echo "Test ${test_num} ended"

############################################
# Test 3: AES256
############################################
test_num=3
echo "Test ${test_num} started"
log_file="flow_results/test${test_num}_run.log"
: > "${log_file}"

LD_PRELOAD=libtracer.so TRACE_OUTPUT=$PWD/flow_results/test${test_num}_trace.log ./test_aes_t3 > "${log_file}" 2>&1
rc=$?

t3_ok=1
if [ $rc -ne 0 ]; then
    t3_ok=0
fi

if ! grep -q "Testing AES256" "${log_file}"; then t3_ok=0; fi
if ! grep -q "CBC encrypt: SUCCESS!" "${log_file}"; then t3_ok=0; fi
if ! grep -q "CBC decrypt: SUCCESS!" "${log_file}"; then t3_ok=0; fi
if ! grep -q "CTR encrypt: SUCCESS!" "${log_file}"; then t3_ok=0; fi
if ! grep -q "CTR decrypt: SUCCESS!" "${log_file}"; then t3_ok=0; fi
if ! grep -q "ECB encrypt: SUCCESS!" "${log_file}"; then t3_ok=0; fi
if ! grep -q "ECB decrypt: SUCCESS!" "${log_file}"; then t3_ok=0; fi
if ! grep -q "ECB encrypt verbose:" "${log_file}"; then t3_ok=0; fi

if [ $t3_ok -eq 1 ]; then
    cp "${log_file}" "flow_results/test${test_num}_success.log"
    echo "Test ${test_num} passed"
else
    cp "${log_file}" "flow_results/test${test_num}_fail.log"
    echo "Test ${test_num} failed" >&2
    failed=1
fi
echo "Test ${test_num} ended"

exit $failed

