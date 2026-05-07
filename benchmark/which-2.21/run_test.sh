
#!/bin/bash

# Reformed test cases

failed=0
mkdir -p flow_results

# Helper: run a testcase
# Args: test_num, test_name, command, expected_output
run_test() {
    local test_num=$1
    local test_name=$2
    local command=$3
    local expected_output=$4

    echo "Test ${test_num} started"

    output=$(eval $command 2>&1)
    exit_code=$?

    # Normalize line endings and remove trailing whitespace
    output=$(echo "$output" | tr -d '\r' | sed 's/[[:space:]]*$//')
    expected_output=$(echo "$expected_output" | tr -d '\r' | sed 's/[[:space:]]*$//')

    # Replace /bin with /usr/bin in expected output and actual output
    expected_output=$(echo "$expected_output" | sed 's|/bin/|/usr/bin/|g')
    output=$(echo "$output" | sed 's|/bin/|/usr/bin/|g')

    local log_content="Test #${test_num}: ${test_name}
Command: ${command}
Expected output: ${expected_output}
Actual output: ${output}
Timestamp: $(date '+%Y-%m-%d %H:%M:%S')"

    if [[ "$output" == *"$expected_output"* ]]; then
        echo "$log_content" > "flow_results/test${test_num}_success.log"
        echo "Test ${test_num} passed"
    else
        echo "$log_content" > "flow_results/test${test_num}_fail.log"
        echo "Test ${test_num} failed" >&2
        echo "Expected output: '$expected_output'" >&2
        echo "Actual output: '$output'" >&2
        failed=1
    fi

    echo "Test ${test_num} ended"
}

# Test 1: Basic functionality
run_test 1 "Basic functionality" "LD_PRELOAD=libtracer.so TRACE_OUTPUT=$PWD/flow_results/test1_trace.log ./which_t1 ls" "/usr/bin/ls"

# Test 2: Non-existent command
run_test 2 "Non-existent command" "LD_PRELOAD=libtracer.so TRACE_OUTPUT=$PWD/flow_results/test2_trace.log ./which_t2 non_existent_command" "./which_t2: no non_existent_command in"

# Test 3: Multiple commands
run_test 3 "Multiple commands" "LD_PRELOAD=libtracer.so TRACE_OUTPUT=$PWD/flow_results/test3_trace.log ./which_t3 ls cat grep" "/usr/bin/ls"$'\n'"/usr/bin/cat"$'\n'"/usr/bin/grep"

# Test 4: All option
run_test 4 "All option" "LD_PRELOAD=libtracer.so TRACE_OUTPUT=$PWD/flow_results/test4_trace.log ./which_t4 -a ls | sort | uniq" "/usr/bin/ls"

# Test 5: Version option
run_test 5 "Version option" "LD_PRELOAD=libtracer.so TRACE_OUTPUT=$PWD/flow_results/test5_trace.log ./which_t5 --version | head -n 4" "GNU which v2.21, Copyright (C) 1999 - 2015 Carlo Wood."$'\n'"GNU which comes with ABSOLUTELY NO WARRANTY;"$'\n'"This program is free software; your freedom to use, change"$'\n'"and distribute this program is protected by the GPL."

# Test 6: Help option
run_test 6 "Help option" "LD_PRELOAD=libtracer.so TRACE_OUTPUT=$PWD/flow_results/test6_trace.log ./which_t6 --help | head -n 1" "Usage: ./which_t6 [options] [--] COMMAND [...]"

# Test 7: Skip dot option
run_test 7 "Skip dot option" "LD_PRELOAD=libtracer.so TRACE_OUTPUT=$PWD/flow_results/test7_trace.log ./which_t7 --skip-dot ls" "/usr/bin/ls"

# Test 8: Skip tilde option
run_test 8 "Skip tilde option" "LD_PRELOAD=libtracer.so TRACE_OUTPUT=$PWD/flow_results/test8_trace.log ./which_t8 --skip-tilde ls" "/usr/bin/ls"

# Test 9: Show dot option
run_test 9 "Show dot option" "LD_PRELOAD=libtracer.so TRACE_OUTPUT=$PWD/flow_results/test9_trace.log ./which_t9 --show-dot ls" "/usr/bin/ls"

# Test 10: Show tilde option
run_test 10 "Show tilde option" "LD_PRELOAD=libtracer.so TRACE_OUTPUT=$PWD/flow_results/test10_trace.log ./which_t10 --show-tilde ls" "/usr/bin/ls"

# Test 11: Multiple non-existent commands
run_test 11 "Multiple non-existent commands" "LD_PRELOAD=libtracer.so TRACE_OUTPUT=$PWD/flow_results/test11_trace.log ./which_t11 non_existent1 non_existent2" "./which_t11: no non_existent1 in"

# Test 12: Skip functions option
run_test 12 "Skip functions option" "LD_PRELOAD=libtracer.so TRACE_OUTPUT=$PWD/flow_results/test12_trace.log ./which_t12 --skip-functions ls" "/usr/bin/ls"

# Test 13: Multiple paths
run_test 13 "Multiple paths" "PATH=/usr/local/bin:/usr/bin LD_PRELOAD=libtracer.so TRACE_OUTPUT=$PWD/flow_results/test13_trace.log ./which_t13 ls" "/usr/bin/ls"

# Test 14: Empty command
run_test 14 "Empty command" "LD_PRELOAD=libtracer.so TRACE_OUTPUT=$PWD/flow_results/test14_trace.log ./which_t14 ''" "./which_t14: no  in"

# Test 15: Commands with multiple spaces
run_test 15 "Commands with multiple spaces" "LD_PRELOAD=libtracer.so TRACE_OUTPUT=$PWD/flow_results/test15_trace.log ./which_t15 cat      ls" "/usr/bin/cat"$'\n'"/usr/bin/ls"

# Test 16: No options with double dash
run_test 16 "No options with double dash" "LD_PRELOAD=libtracer.so TRACE_OUTPUT=$PWD/flow_results/test16_trace.log ./which_t16 -- ls" "/usr/bin/ls"

# Test 17: Multiple options
run_test 17 "Multiple options" "LD_PRELOAD=libtracer.so TRACE_OUTPUT=$PWD/flow_results/test17_trace.log ./which_t17 -a ls 2>&1 | grep -c /ls" "2"

# Test 18: Invalid option
run_test 18 "Invalid option" "LD_PRELOAD=libtracer.so TRACE_OUTPUT=$PWD/flow_results/test18_trace.log ./which_t18 --invalid-option" "./which_t18: unrecognized option '--invalid-option'"$'\n'"Usage: ./which_t18 [options] [--] COMMAND [...]"$'\n'"Write the full path of COMMAND(s) to standard output."

# Test 19: Directory command
run_test 19 "Directory command" "LD_PRELOAD=libtracer.so TRACE_OUTPUT=$PWD/flow_results/test19_trace.log ./which_t19 /usr/bin" "./which_t19: no bin in (/usr)"

# Test 20: Path with spaces
run_test 20 "Path with spaces" "LD_PRELOAD=libtracer.so TRACE_OUTPUT=$PWD/flow_results/test20_trace.log ./which_t20 \"program with spaces\"" "./which_t20: no program with spaces in"

exit $failed

