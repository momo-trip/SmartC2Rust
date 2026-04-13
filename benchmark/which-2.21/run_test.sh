
#!/bin/bash

# Reformed test cases
failed=0

# Create results directory
mkdir -p flow_results

# Helper: normalize output (remove \r, trailing whitespace, /bin/ -> /usr/bin/)
normalize() {
    echo "$1" | tr -d '\r' | sed 's/[[:space:]]*$//' | sed 's|/bin/|/usr/bin/|g'
}

###############################################################################
# Test 1: Basic functionality
###############################################################################
test_num=1
echo "Test ${test_num} started"
log=""
expected="/usr/bin/ls"
expected=$(normalize "$expected")
output=$(LD_PRELOAD=libtracer.so TRACE_OUTPUT=$PWD/flow_results/test${test_num}_trace.log ./which_t${test_num} ls 2>&1)
exit_code=$?
output=$(normalize "$output")
log="Test #${test_num}: Basic functionality\nCommand: ./which_t${test_num} ls\nExpected output: ${expected}\nActual output: ${output}\nExit code: ${exit_code}"
if [[ "$output" == *"$expected"* ]]; then
    echo "Test ${test_num} passed"
    echo -e "$log\nStatus: PASSED" > flow_results/test${test_num}_success.log
else
    echo "Test ${test_num} failed"
    echo "Test ${test_num} failed" >&2
    echo -e "$log\nStatus: FAILED" > flow_results/test${test_num}_fail.log
    failed=1
fi
echo "Test ${test_num} ended"

###############################################################################
# Test 2: Non-existent command
###############################################################################
test_num=2
echo "Test ${test_num} started"
log=""
expected="./which_t${test_num}: no non_existent_command in"
output=$(LD_PRELOAD=libtracer.so TRACE_OUTPUT=$PWD/flow_results/test${test_num}_trace.log ./which_t${test_num} non_existent_command 2>&1)
exit_code=$?
output=$(normalize "$output")
expected=$(normalize "$expected")
log="Test #${test_num}: Non-existent command\nCommand: ./which_t${test_num} non_existent_command\nExpected output: ${expected}\nActual output: ${output}\nExit code: ${exit_code}"
if [[ "$output" == *"no non_existent_command in"* ]]; then
    echo "Test ${test_num} passed"
    echo -e "$log\nStatus: PASSED" > flow_results/test${test_num}_success.log
else
    echo "Test ${test_num} failed"
    echo "Test ${test_num} failed" >&2
    echo -e "$log\nStatus: FAILED" > flow_results/test${test_num}_fail.log
    failed=1
fi
echo "Test ${test_num} ended"

###############################################################################
# Test 3: Multiple commands
###############################################################################
test_num=3
echo "Test ${test_num} started"
log=""
expected_ls="/usr/bin/ls"
expected_cat="/usr/bin/cat"
expected_grep="/usr/bin/grep"
output=$(LD_PRELOAD=libtracer.so TRACE_OUTPUT=$PWD/flow_results/test${test_num}_trace.log ./which_t${test_num} ls cat grep 2>&1)
exit_code=$?
output=$(normalize "$output")
log="Test #${test_num}: Multiple commands\nCommand: ./which_t${test_num} ls cat grep\nExpected: ls,cat,grep paths\nActual output: ${output}\nExit code: ${exit_code}"
if [[ "$output" == *"$expected_ls"* ]] && [[ "$output" == *"$expected_cat"* ]] && [[ "$output" == *"$expected_grep"* ]]; then
    echo "Test ${test_num} passed"
    echo -e "$log\nStatus: PASSED" > flow_results/test${test_num}_success.log
else
    echo "Test ${test_num} failed"
    echo "Test ${test_num} failed" >&2
    echo -e "$log\nStatus: FAILED" > flow_results/test${test_num}_fail.log
    failed=1
fi
echo "Test ${test_num} ended"

###############################################################################
# Test 4: All option
###############################################################################
test_num=4
echo "Test ${test_num} started"
log=""
expected="/usr/bin/ls"
output=$(LD_PRELOAD=libtracer.so TRACE_OUTPUT=$PWD/flow_results/test${test_num}_trace.log ./which_t${test_num} -a ls 2>&1 | sort | uniq)
exit_code=$?
output=$(normalize "$output")
expected=$(normalize "$expected")
log="Test #${test_num}: All option\nCommand: ./which_t${test_num} -a ls | sort | uniq\nExpected output: ${expected}\nActual output: ${output}\nExit code: ${exit_code}"
if [[ "$output" == *"$expected"* ]]; then
    echo "Test ${test_num} passed"
    echo -e "$log\nStatus: PASSED" > flow_results/test${test_num}_success.log
else
    echo "Test ${test_num} failed"
    echo "Test ${test_num} failed" >&2
    echo -e "$log\nStatus: FAILED" > flow_results/test${test_num}_fail.log
    failed=1
fi
echo "Test ${test_num} ended"

###############################################################################
# Test 5: Version option
###############################################################################
test_num=5
echo "Test ${test_num} started"
log=""
expected="GNU which v2.21, Copyright (C) 1999 - 2015 Carlo Wood."
output=$(LD_PRELOAD=libtracer.so TRACE_OUTPUT=$PWD/flow_results/test${test_num}_trace.log ./which_t${test_num} --version 2>&1 | head -n 4)
exit_code=$?
output=$(normalize "$output")
expected=$(normalize "$expected")
log="Test #${test_num}: Version option\nCommand: ./which_t${test_num} --version | head -n 4\nExpected output: ${expected}\nActual output: ${output}\nExit code: ${exit_code}"
if [[ "$output" == *"$expected"* ]]; then
    echo "Test ${test_num} passed"
    echo -e "$log\nStatus: PASSED" > flow_results/test${test_num}_success.log
else
    echo "Test ${test_num} failed"
    echo "Test ${test_num} failed" >&2
    echo -e "$log\nStatus: FAILED" > flow_results/test${test_num}_fail.log
    failed=1
fi
echo "Test ${test_num} ended"

###############################################################################
# Test 6: Help option
###############################################################################
test_num=6
echo "Test ${test_num} started"
log=""
expected="Usage: ./which_t${test_num} [options] [--] COMMAND [...]"
output=$(LD_PRELOAD=libtracer.so TRACE_OUTPUT=$PWD/flow_results/test${test_num}_trace.log ./which_t${test_num} --help 2>&1 | head -n 1)
exit_code=$?
output=$(normalize "$output")
expected=$(normalize "$expected")
log="Test #${test_num}: Help option\nCommand: ./which_t${test_num} --help | head -n 1\nExpected output: ${expected}\nActual output: ${output}\nExit code: ${exit_code}"
if [[ "$output" == *"[options] [--] COMMAND"* ]]; then
    echo "Test ${test_num} passed"
    echo -e "$log\nStatus: PASSED" > flow_results/test${test_num}_success.log
else
    echo "Test ${test_num} failed"
    echo "Test ${test_num} failed" >&2
    echo -e "$log\nStatus: FAILED" > flow_results/test${test_num}_fail.log
    failed=1
fi
echo "Test ${test_num} ended"

###############################################################################
# Test 7: Skip dot option
###############################################################################
test_num=7
echo "Test ${test_num} started"
log=""
expected="/usr/bin/ls"
output=$(LD_PRELOAD=libtracer.so TRACE_OUTPUT=$PWD/flow_results/test${test_num}_trace.log ./which_t${test_num} --skip-dot ls 2>&1)
exit_code=$?
output=$(normalize "$output")
expected=$(normalize "$expected")
log="Test #${test_num}: Skip dot option\nCommand: ./which_t${test_num} --skip-dot ls\nExpected output: ${expected}\nActual output: ${output}\nExit code: ${exit_code}"
if [[ "$output" == *"$expected"* ]]; then
    echo "Test ${test_num} passed"
    echo -e "$log\nStatus: PASSED" > flow_results/test${test_num}_success.log
else
    echo "Test ${test_num} failed"
    echo "Test ${test_num} failed" >&2
    echo -e "$log\nStatus: FAILED" > flow_results/test${test_num}_fail.log
    failed=1
fi
echo "Test ${test_num} ended"

###############################################################################
# Test 8: Skip tilde option
###############################################################################
test_num=8
echo "Test ${test_num} started"
log=""
expected="/usr/bin/ls"
output=$(LD_PRELOAD=libtracer.so TRACE_OUTPUT=$PWD/flow_results/test${test_num}_trace.log ./which_t${test_num} --skip-tilde ls 2>&1)
exit_code=$?
output=$(normalize "$output")
expected=$(normalize "$expected")
log="Test #${test_num}: Skip tilde option\nCommand: ./which_t${test_num} --skip-tilde ls\nExpected output: ${expected}\nActual output: ${output}\nExit code: ${exit_code}"
if [[ "$output" == *"$expected"* ]]; then
    echo "Test ${test_num} passed"
    echo -e "$log\nStatus: PASSED" > flow_results/test${test_num}_success.log
else
    echo "Test ${test_num} failed"
    echo "Test ${test_num} failed" >&2
    echo -e "$log\nStatus: FAILED" > flow_results/test${test_num}_fail.log
    failed=1
fi
echo "Test ${test_num} ended"

###############################################################################
# Test 9: Show dot option
###############################################################################
test_num=9
echo "Test ${test_num} started"
log=""
expected="/usr/bin/ls"
output=$(LD_PRELOAD=libtracer.so TRACE_OUTPUT=$PWD/flow_results/test${test_num}_trace.log ./which_t${test_num} --show-dot ls 2>&1)
exit_code=$?
output=$(normalize "$output")
expected=$(normalize "$expected")
log="Test #${test_num}: Show dot option\nCommand: ./which_t${test_num} --show-dot ls\nExpected output: ${expected}\nActual output: ${output}\nExit code: ${exit_code}"
if [[ "$output" == *"$expected"* ]]; then
    echo "Test ${test_num} passed"
    echo -e "$log\nStatus: PASSED" > flow_results/test${test_num}_success.log
else
    echo "Test ${test_num} failed"
    echo "Test ${test_num} failed" >&2
    echo -e "$log\nStatus: FAILED" > flow_results/test${test_num}_fail.log
    failed=1
fi
echo "Test ${test_num} ended"

###############################################################################
# Test 10: Show tilde option
###############################################################################
test_num=10
echo "Test ${test_num} started"
log=""
expected="/usr/bin/ls"
output=$(LD_PRELOAD=libtracer.so TRACE_OUTPUT=$PWD/flow_results/test${test_num}_trace.log ./which_t${test_num} --show-tilde ls 2>&1)
exit_code=$?
output=$(normalize "$output")
expected=$(normalize "$expected")
log="Test #${test_num}: Show tilde option\nCommand: ./which_t${test_num} --show-tilde ls\nExpected output: ${expected}\nActual output: ${output}\nExit code: ${exit_code}"
if [[ "$output" == *"$expected"* ]]; then
    echo "Test ${test_num} passed"
    echo -e "$log\nStatus: PASSED" > flow_results/test${test_num}_success.log
else
    echo "Test ${test_num} failed"
    echo "Test ${test_num} failed" >&2
    echo -e "$log\nStatus: FAILED" > flow_results/test${test_num}_fail.log
    failed=1
fi
echo "Test ${test_num} ended"

###############################################################################
# Test 11: Multiple non-existent commands
###############################################################################
test_num=11
echo "Test ${test_num} started"
log=""
output=$(LD_PRELOAD=libtracer.so TRACE_OUTPUT=$PWD/flow_results/test${test_num}_trace.log ./which_t${test_num} non_existent1 non_existent2 2>&1)
exit_code=$?
output=$(normalize "$output")
log="Test #${test_num}: Multiple non-existent commands\nCommand: ./which_t${test_num} non_existent1 non_existent2\nActual output: ${output}\nExit code: ${exit_code}"
if [[ "$output" == *"no non_existent1 in"* ]]; then
    echo "Test ${test_num} passed"
    echo -e "$log\nStatus: PASSED" > flow_results/test${test_num}_success.log
else
    echo "Test ${test_num} failed"
    echo "Test ${test_num} failed" >&2
    echo -e "$log\nStatus: FAILED" > flow_results/test${test_num}_fail.log
    failed=1
fi
echo "Test ${test_num} ended"

###############################################################################
# Test 12: Skip functions option
###############################################################################
test_num=12
echo "Test ${test_num} started"
log=""
expected="/usr/bin/ls"
output=$(LD_PRELOAD=libtracer.so TRACE_OUTPUT=$PWD/flow_results/test${test_num}_trace.log ./which_t${test_num} --skip-functions ls 2>&1)
exit_code=$?
output=$(normalize "$output")
expected=$(normalize "$expected")
log="Test #${test_num}: Skip functions option\nCommand: ./which_t${test_num} --skip-functions ls\nExpected output: ${expected}\nActual output: ${output}\nExit code: ${exit_code}"
if [[ "$output" == *"$expected"* ]]; then
    echo "Test ${test_num} passed"
    echo -e "$log\nStatus: PASSED" > flow_results/test${test_num}_success.log
else
    echo "Test ${test_num} failed"
    echo "Test ${test_num} failed" >&2
    echo -e "$log\nStatus: FAILED" > flow_results/test${test_num}_fail.log
    failed=1
fi
echo "Test ${test_num} ended"

###############################################################################
# Test 13: Multiple paths
###############################################################################
test_num=13
echo "Test ${test_num} started"
log=""
expected="/usr/bin/ls"
output=$(PATH=/usr/local/bin:/usr/bin LD_PRELOAD=libtracer.so TRACE_OUTPUT=$PWD/flow_results/test${test_num}_trace.log ./which_t${test_num} ls 2>&1)
exit_code=$?
output=$(normalize "$output")
expected=$(normalize "$expected")
log="Test #${test_num}: Multiple paths\nCommand: PATH=/usr/local/bin:/usr/bin ./which_t${test_num} ls\nExpected output: ${expected}\nActual output: ${output}\nExit code: ${exit_code}"
if [[ "$output" == *"$expected"* ]]; then
    echo "Test ${test_num} passed"
    echo -e "$log\nStatus: PASSED" > flow_results/test${test_num}_success.log
else
    echo "Test ${test_num} failed"
    echo "Test ${test_num} failed" >&2
    echo -e "$log\nStatus: FAILED" > flow_results/test${test_num}_fail.log
    failed=1
fi
echo "Test ${test_num} ended"

###############################################################################
# Test 14: Empty command
###############################################################################
test_num=14
echo "Test ${test_num} started"
log=""
output=$(LD_PRELOAD=libtracer.so TRACE_OUTPUT=$PWD/flow_results/test${test_num}_trace.log ./which_t${test_num} '' 2>&1)
exit_code=$?
output=$(normalize "$output")
log="Test #${test_num}: Empty command\nCommand: ./which_t${test_num} ''\nActual output: ${output}\nExit code: ${exit_code}"
if [[ "$output" == *"no  in"* ]]; then
    echo "Test ${test_num} passed"
    echo -e "$log\nStatus: PASSED" > flow_results/test${test_num}_success.log
else
    echo "Test ${test_num} failed"
    echo "Test ${test_num} failed" >&2
    echo -e "$log\nStatus: FAILED" > flow_results/test${test_num}_fail.log
    failed=1
fi
echo "Test ${test_num} ended"

###############################################################################
# Test 15: Commands with multiple spaces
###############################################################################
test_num=15
echo "Test ${test_num} started"
log=""
expected_cat="/usr/bin/cat"
expected_ls="/usr/bin/ls"
output=$(LD_PRELOAD=libtracer.so TRACE_OUTPUT=$PWD/flow_results/test${test_num}_trace.log ./which_t${test_num} cat ls 2>&1)
exit_code=$?
output=$(normalize "$output")
log="Test #${test_num}: Commands with multiple spaces\nCommand: ./which_t${test_num} cat ls\nActual output: ${output}\nExit code: ${exit_code}"
if [[ "$output" == *"$expected_cat"* ]] && [[ "$output" == *"$expected_ls"* ]]; then
    echo "Test ${test_num} passed"
    echo -e "$log\nStatus: PASSED" > flow_results/test${test_num}_success.log
else
    echo "Test ${test_num} failed"
    echo "Test ${test_num} failed" >&2
    echo -e "$log\nStatus: FAILED" > flow_results/test${test_num}_fail.log
    failed=1
fi
echo "Test ${test_num} ended"

###############################################################################
# Test 16: No options with double dash
###############################################################################
test_num=16
echo "Test ${test_num} started"
log=""
expected="/usr/bin/ls"
output=$(LD_PRELOAD=libtracer.so TRACE_OUTPUT=$PWD/flow_results/test${test_num}_trace.log ./which_t${test_num} -- ls 2>&1)
exit_code=$?
output=$(normalize "$output")
expected=$(normalize "$expected")
log="Test #${test_num}: No options with double dash\nCommand: ./which_t${test_num} -- ls\nExpected output: ${expected}\nActual output: ${output}\nExit code: ${exit_code}"
if [[ "$output" == *"$expected"* ]]; then
    echo "Test ${test_num} passed"
    echo -e "$log\nStatus: PASSED" > flow_results/test${test_num}_success.log
else
    echo "Test ${test_num} failed"
    echo "Test ${test_num} failed" >&2
    echo -e "$log\nStatus: FAILED" > flow_results/test${test_num}_fail.log
    failed=1
fi
echo "Test ${test_num} ended"

###############################################################################
# Test 17: Multiple options (-a with grep count)
###############################################################################
test_num=17
echo "Test ${test_num} started"
log=""
expected="2"
output=$(LD_PRELOAD=libtracer.so TRACE_OUTPUT=$PWD/flow_results/test${test_num}_trace.log ./which_t${test_num} -a ls 2>&1 | grep -c /ls)
exit_code=$?
output=$(normalize "$output")
expected=$(normalize "$expected")
log="Test #${test_num}: Multiple options\nCommand: ./which_t${test_num} -a ls 2>&1 | grep -c /ls\nExpected output: ${expected}\nActual output: ${output}\nExit code: ${exit_code}"
if [[ "$output" == *"$expected"* ]]; then
    echo "Test ${test_num} passed"
    echo -e "$log\nStatus: PASSED" > flow_results/test${test_num}_success.log
else
    echo "Test ${test_num} failed"
    echo "Test ${test_num} failed" >&2
    echo -e "$log\nStatus: FAILED" > flow_results/test${test_num}_fail.log
    failed=1
fi
echo "Test ${test_num} ended"

###############################################################################
# Test 18: Invalid option
###############################################################################
test_num=18
echo "Test ${test_num} started"
log=""
output=$(LD_PRELOAD=libtracer.so TRACE_OUTPUT=$PWD/flow_results/test${test_num}_trace.log ./which_t${test_num} --invalid-option 2>&1)
exit_code=$?
output=$(normalize "$output")
log="Test #${test_num}: Invalid option\nCommand: ./which_t${test_num} --invalid-option\nActual output: ${output}\nExit code: ${exit_code}"
if [[ "$output" == *"unrecognized option"* ]] && [[ "$output" == *"Usage:"* ]]; then
    echo "Test ${test_num} passed"
    echo -e "$log\nStatus: PASSED" > flow_results/test${test_num}_success.log
else
    echo "Test ${test_num} failed"
    echo "Test ${test_num} failed" >&2
    echo -e "$log\nStatus: FAILED" > flow_results/test${test_num}_fail.log
    failed=1
fi
echo "Test ${test_num} ended"

###############################################################################
# Test 19: Directory command
###############################################################################
test_num=19
echo "Test ${test_num} started"
log=""
output=$(LD_PRELOAD=libtracer.so TRACE_OUTPUT=$PWD/flow_results/test${test_num}_trace.log ./which_t${test_num} /usr/bin 2>&1)
exit_code=$?
output=$(normalize "$output")
log="Test #${test_num}: Directory command\nCommand: ./which_t${test_num} /usr/bin\nActual output: ${output}\nExit code: ${exit_code}"
if [[ "$output" == *"no bin in"* ]] || [[ "$output" == *"no /usr/bin in"* ]] || [[ $exit_code -ne 0 ]]; then
    echo "Test ${test_num} passed"
    echo -e "$log\nStatus: PASSED" > flow_results/test${test_num}_success.log
else
    echo "Test ${test_num} failed"
    echo "Test ${test_num} failed" >&2
    echo -e "$log\nStatus: FAILED" > flow_results/test${test_num}_fail.log
    failed=1
fi
echo "Test ${test_num} ended"

###############################################################################
# Test 20: Path with spaces
###############################################################################
test_num=20
echo "Test ${test_num} started"
log=""
output=$(LD_PRELOAD=libtracer.so TRACE_OUTPUT=$PWD/flow_results/test${test_num}_trace.log ./which_t${test_num} "program with spaces" 2>&1)
exit_code=$?
output=$(normalize "$output")
log="Test #${test_num}: Path with spaces\nCommand: ./which_t${test_num} 'program with spaces'\nActual output: ${output}\nExit code: ${exit_code}"
if [[ "$output" == *"no program with spaces in"* ]]; then
    echo "Test ${test_num} passed"
    echo -e "$log\nStatus: PASSED" > flow_results/test${test_num}_success.log
else
    echo "Test ${test_num} failed"
    echo "Test ${test_num} failed" >&2
    echo -e "$log\nStatus: FAILED" > flow_results/test${test_num}_fail.log
    failed=1
fi
echo "Test ${test_num} ended"

###############################################################################
# Final result
###############################################################################
echo "Tests completed. failed=${failed}"
exit $failed

