
#!/bin/bash

# Reformed test cases

failed=0
mkdir -p flow_results

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$SCRIPT_DIR"

run_tc() {
    local test_num=$1
    local test_name=$2
    local command=$3
    local expected_output=$4

    echo "Test ${test_num} started"

    local log_file_success="flow_results/test${test_num}_success.log"
    local log_file_fail="flow_results/test${test_num}_fail.log"

    output=$(eval "LD_PRELOAD=libtracer.so TRACE_OUTPUT=$PWD/flow_results/test${test_num}_trace.log $command" 2>&1)
    exit_code=$?

    output=$(echo "$output" | tr -d '\r' | sed 's/[[:space:]]*$//')
    expected_output=$(echo "$expected_output" | tr -d '\r' | sed 's/[[:space:]]*$//')
    expected_output=$(echo "$expected_output" | sed 's|/bin/|/usr/bin/|g')
    output=$(echo "$output" | sed 's|/bin/|/usr/bin/|g')

    local log_content="Test #${test_num}: ${test_name}
Command: ${command}
Expected output: ${expected_output}
Actual output: ${output}
Exit Code: ${exit_code}
Timestamp: $(date '+%Y-%m-%d %H:%M:%S')"

    if [[ "$output" == *"$expected_output"* ]]; then
        echo "$log_content" > "$log_file_success"
        echo "Test ${test_num} passed"
    else
        echo "$log_content" > "$log_file_fail"
        echo "Test ${test_num} failed" >&2
        failed=1
    fi

    echo "Test ${test_num} ended"
}

# Prepare unique binary copies per testcase to avoid "Text file busy"
for i in $(seq 1 20); do
    if [ ! -f "./which_t${i}" ] && [ -f "./which" ]; then
        cp ./which ./which_t${i}
    fi
done

run_tc 1 "Basic functionality" "./which_t1 ls" "/usr/bin/ls"
run_tc 2 "Non-existent command" "./which_t2 non_existent_command" "./which_t2: no non_existent_command in"
run_tc 3 "Multiple commands" "./which_t3 ls cat grep" "/usr/bin/ls"$'\n'"/usr/bin/cat"$'\n'"/usr/bin/grep"
run_tc 4 "All option" "./which_t4 -a ls | sort | uniq" "/usr/bin/ls"
run_tc 5 "Version option" "./which_t5 --version | head -n 4" "GNU which v2.21, Copyright (C) 1999 - 2015 Carlo Wood."$'\n'"GNU which comes with ABSOLUTELY NO WARRANTY;"$'\n'"This program is free software; your freedom to use, change"$'\n'"and distribute this program is protected by the GPL."
run_tc 6 "Help option" "./which_t6 --help | head -n 1" "Usage: ./which_t6 [options] [--] COMMAND [...]"
run_tc 7 "Skip dot option" "./which_t7 --skip-dot ls" "/usr/bin/ls"
run_tc 8 "Skip tilde option" "./which_t8 --skip-tilde ls" "/usr/bin/ls"
run_tc 9 "Show dot option" "./which_t9 --show-dot ls" "/usr/bin/ls"
run_tc 10 "Show tilde option" "./which_t10 --show-tilde ls" "/usr/bin/ls"
run_tc 11 "Multiple non-existent commands" "./which_t11 non_existent1 non_existent2" "./which_t11: no non_existent1 in"
run_tc 12 "Skip functions option" "./which_t12 --skip-functions ls" "/usr/bin/ls"
run_tc 13 "Multiple paths" "PATH=/usr/local/bin:/usr/bin ./which_t13 ls" "/usr/bin/ls"
run_tc 14 "Empty command" "./which_t14 ''" "./which_t14: no  in"
run_tc 15 "Commands with multiple spaces" "./which_t15 cat      ls" "/usr/bin/cat"$'\n'"/usr/bin/ls"
run_tc 16 "No options with double dash" "./which_t16 -- ls" "/usr/bin/ls"
run_tc 17 "Multiple options" "./which_t17 -a ls 2>&1 | grep -c /ls" "2"
run_tc 18 "Invalid option" "./which_t18 --invalid-option" "./which_t18: unrecognized option '--invalid-option'"$'\n'"Usage: ./which_t18 [options] [--] COMMAND [...]"$'\n'"Write the full path of COMMAND(s) to standard output."
run_tc 19 "Directory command" "./which_t19 /usr/bin" "./which_t19: no bin in (/usr)"
run_tc 20 "Path with spaces" "./which_t20 \"program with spaces\"" "./which_t20: no program with spaces in"

exit $failed

