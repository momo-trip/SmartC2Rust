#!/bin/bash

# Initialize variables
total_tests=0
passed_tests=0

# Create results directory if it doesn't exist
rm -rf results
mkdir -p results

# Function to run a test case
run_test() {
    local test_name=$1
    local command=$2
    local expected_output=$3
    local expected_exit_code=$4
    local current_test=$((total_tests + 1))
    local log_file=""
    local test_passed=false

    echo "Running test #${current_test}: $test_name"
    echo "Test Case #$current_test: Started" >> /home/ubuntu/portable/out_flow_c.log
    echo "Test Case #$current_test: Started" >> /home/ubuntu/portable/out_flow_rust.log
    output=$(eval $command 2>&1)
    exit_code=$?

    # Normalize line endings and remove trailing whitespace
    output=$(echo "$output" | tr -d '\r' | sed 's/[[:space:]]*$//')
    expected_output=$(echo "$expected_output" | tr -d '\r' | sed 's/[[:space:]]*$//')

    # Replace /bin with /usr/bin in expected output and actual output
    expected_output=$(echo "$expected_output" | sed 's|/bin/|/usr/bin/|g')
    output=$(echo "$output" | sed 's|/bin/|/usr/bin/|g')

    # Function to write log
    write_log() {
        local status=$1
        local log_content="Test #${current_test}: ${test_name}
Command: ${command}
Expected output: ${expected_output}
Actual output: ${output}
Status: ${status}
Timestamp: $(date '+%Y-%m-%d %H:%M:%S')"

        if [ "$status" = "PASSED" ]; then
            log_file="results/test${current_test}_success.log"
        else
            log_file="results/test${current_test}_fail.log"
        fi

        echo "$log_content" > "$log_file"
    }
    if [[ "$output" == *"$expected_output"* ]]; then
        echo "Test #${current_test} passed"
        write_log "PASSED"
        passed_tests=$((passed_tests + 1))
    else
        echo "Test #${current_test} failed"
        echo "Expected output: '$expected_output'"
        echo "Actual output: '$output'"
        write_log "FAILED"
    fi
    total_tests=$((total_tests + 1))
    echo
}

# Test cases
run_test "Basic functionality" "./which ls" "/usr/bin/ls" 0
run_test "Non-existent command" "./which non_existent_command" "./which: no non_existent_command in" 1
run_test "Multiple commands" "./which ls cat grep" "/usr/bin/ls"$'\n'"/usr/bin/cat"$'\n'"/usr/bin/grep" 0
run_test "All option" "./which -a ls | sort | uniq" "/usr/bin/ls" 0
run_test "Version option" "./which --version | head -n 4" "GNU which v2.21, Copyright (C) 1999 - 2015 Carlo Wood."$'\n'"GNU which comes with ABSOLUTELY NO WARRANTY;"$'\n'"This program is free software; your freedom to use, change"$'\n'"and distribute this program is protected by the GPL." 0

run_test "Help option" "./which --help | head -n 1" "Usage: ./which [options] [--] COMMAND [...]" 0
run_test "Skip dot option" "./which --skip-dot ls" "/usr/bin/ls" 0
run_test "Skip tilde option" "./which --skip-tilde ls" "/usr/bin/ls" 0
run_test "Show dot option" "./which --show-dot ls" "/usr/bin/ls" 0
run_test "Show tilde option" "./which --show-tilde ls" "/usr/bin/ls" 0

# Additional test cases
run_test "Multiple non-existent commands" "./which non_existent1 non_existent2" "./which: no non_existent1 in" 1
run_test "Skip functions option" "./which --skip-functions ls" "/usr/bin/ls" 0
run_test "Multiple paths" "PATH=/usr/local/bin:/usr/bin ./which ls" "/usr/bin/ls" 0
run_test "Empty command" "./which ''" "./which: no  in" 1
run_test "Commands with multiple spaces" "./which cat      ls" "/usr/bin/cat"$'\n'"/usr/bin/ls" 0
run_test "No options with double dash" "./which -- ls" "/usr/bin/ls" 0
run_test "Multiple options" "./which -a ls 2>&1 | grep -c /ls" "2" 0
run_test "Invalid option" "./which --invalid-option" "./which: unrecognized option '--invalid-option'"$'\n'"Usage: ./which [options] [--] COMMAND [...]"$'\n'"Write the full path of COMMAND(s) to standard output." 1
run_test "Directory command" "./which /usr/bin" "./which: no bin in (/usr)" 1
run_test "Path with spaces" "./which \"program with spaces\"" "./which: no program with spaces in" 1

# Calculate pass rate
pass_rate=$((passed_tests * 100 / total_tests))

echo "Tests passed: $passed_tests/$total_tests"
echo "Pass rate: $pass_rate%"

if [ $pass_rate -eq 100 ]; then
    echo "All tests passed successfully."
    exit 0
else
    echo "Some tests failed." >&2
    echo "Failed test cases:" >&2
    for ((i=1; i<=total_tests; i++)); do
        if ! grep -q "Test Case #$i: Started" /home/ubuntu/portable/out_flow_c.log; then
            echo "Test Case #$i failed" >&2
        fi
    done
    exit 1
fi