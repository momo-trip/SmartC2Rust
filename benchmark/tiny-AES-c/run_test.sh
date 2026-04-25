
#!/bin/bash

# Reformed test cases

mkdir -p flow_results

failed=0

# -------------------------------------------------------------------
# Test 1: Default AES (AES128) build, run ./test_aes128.elf
# -------------------------------------------------------------------
test_num=1
binary=test_aes128.elf
echo "Test ${test_num} started"

if [ ! -x "./${binary}" ]; then
    echo "Test ${test_num} failed" >&2
    echo "Binary ./${binary} not found or not executable" > "flow_results/test${test_num}_fail.log"
    echo "Test ${test_num} ended"
    failed=1
else
    tmp_out="flow_results/test${test_num}_output.log"
    LD_PRELOAD=libtracer.so TRACE_OUTPUT=$PWD/flow_results/test${test_num}_trace.log ./${binary} > "${tmp_out}" 2>&1
    rc=$?

    # Validate: expect an AES128 run with SUCCESS messages
    if [ $rc -eq 0 ] && grep -q "Testing AES128" "${tmp_out}" \
        && grep -q "CBC encrypt: SUCCESS" "${tmp_out}" \
        && grep -q "CBC decrypt: SUCCESS" "${tmp_out}" \
        && grep -q "ECB encrypt: SUCCESS" "${tmp_out}"; then
        echo "Test ${test_num} passed"
        cp "${tmp_out}" "flow_results/test${test_num}_success.log"
        echo "Test ${test_num} ended"
    else
        echo "Test ${test_num} failed" >&2
        cp "${tmp_out}" "flow_results/test${test_num}_fail.log"
        echo "Test ${test_num} ended"
        failed=1
    fi
fi

# -------------------------------------------------------------------
# Test 2: AES192 build, run ./test_aes192.elf
# -------------------------------------------------------------------
test_num=2
binary=test_aes192.elf
echo "Test ${test_num} started"

if [ ! -x "./${binary}" ]; then
    echo "Test ${test_num} failed" >&2
    echo "Binary ./${binary} not found or not executable" > "flow_results/test${test_num}_fail.log"
    echo "Test ${test_num} ended"
    failed=1
else
    tmp_out="flow_results/test${test_num}_output.log"
    LD_PRELOAD=libtracer.so TRACE_OUTPUT=$PWD/flow_results/test${test_num}_trace.log ./${binary} > "${tmp_out}" 2>&1
    rc=$?

    if [ $rc -eq 0 ] && grep -q "Testing AES192" "${tmp_out}" \
        && grep -q "CBC encrypt: SUCCESS" "${tmp_out}" \
        && grep -q "CBC decrypt: SUCCESS" "${tmp_out}" \
        && grep -q "ECB encrypt: SUCCESS" "${tmp_out}"; then
        echo "Test ${test_num} passed"
        cp "${tmp_out}" "flow_results/test${test_num}_success.log"
        echo "Test ${test_num} ended"
    else
        echo "Test ${test_num} failed" >&2
        cp "${tmp_out}" "flow_results/test${test_num}_fail.log"
        echo "Test ${test_num} ended"
        failed=1
    fi
fi

# -------------------------------------------------------------------
# Test 3: AES256 build, run ./test_aes256.elf
# -------------------------------------------------------------------
test_num=3
binary=test_aes256.elf
echo "Test ${test_num} started"

if [ ! -x "./${binary}" ]; then
    echo "Test ${test_num} failed" >&2
    echo "Binary ./${binary} not found or not executable" > "flow_results/test${test_num}_fail.log"
    echo "Test ${test_num} ended"
    failed=1
else
    tmp_out="flow_results/test${test_num}_output.log"
    LD_PRELOAD=libtracer.so TRACE_OUTPUT=$PWD/flow_results/test${test_num}_trace.log ./${binary} > "${tmp_out}" 2>&1
    rc=$?

    if [ $rc -eq 0 ] && grep -q "Testing AES256" "${tmp_out}" \
        && grep -q "CBC encrypt: SUCCESS" "${tmp_out}" \
        && grep -q "CBC decrypt: SUCCESS" "${tmp_out}" \
        && grep -q "ECB encrypt: SUCCESS" "${tmp_out}"; then
        echo "Test ${test_num} passed"
        cp "${tmp_out}" "flow_results/test${test_num}_success.log"
        echo "Test ${test_num} ended"
    else
        echo "Test ${test_num} failed" >&2
        cp "${tmp_out}" "flow_results/test${test_num}_fail.log"
        echo "Test ${test_num} ended"
        failed=1
    fi
fi

exit $failed

