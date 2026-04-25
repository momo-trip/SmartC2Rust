
#!/bin/bash

# Reformed test cases

failed=0
mkdir -p flow_results
mkdir -p actual

EXPECTED_DIR="expected"
ACTUAL_DIR="actual"

run_case() {
    local test_num="$1"
    local test_name="$2"
    local binary="$3"
    shift 3
    local args=("$@")

    echo "Test ${test_num} started"

    local actual_stdout="$ACTUAL_DIR/${test_name}_stdout.txt"
    local actual_stderr_raw="$ACTUAL_DIR/${test_name}_stderr_raw.txt"
    local actual_stderr="$ACTUAL_DIR/${test_name}_stderr.txt"
    local actual_exitcode="$ACTUAL_DIR/${test_name}_exitcode.txt"
    local expected_stdout="$EXPECTED_DIR/${test_name}_stdout.txt"
    local expected_stderr="$EXPECTED_DIR/${test_name}_stderr.txt"
    local expected_exitcode="$EXPECTED_DIR/${test_name}_exitcode.txt"
    local log_tmp="flow_results/test${test_num}_tmp.log"

    LD_PRELOAD=libtracer.so TRACE_OUTPUT="$PWD/flow_results/test${test_num}_trace.log" "./${binary}" "${args[@]}" > "$actual_stdout" 2> "$actual_stderr_raw"
    echo "$?" > "$actual_exitcode"

    # Normalize binary name in output to match expected (which uses 'su-exec')
    sed -e "s|\./${binary}|./su-exec|g" -e "s|${binary}:|su-exec:|g" "$actual_stdout" > "${actual_stdout}.norm"
    mv "${actual_stdout}.norm" "$actual_stdout"

    # Strip tracer messages and normalize binary name in stderr
    grep -v '^\[tracer\]' "$actual_stderr_raw" | sed -e "s|\./${binary}|./su-exec|g" -e "s|${binary}:|su-exec:|g" > "$actual_stderr"

    local test_passed=true
    local differences=""

    if [ ! -f "$expected_stdout" ] || [ ! -f "$expected_stderr" ] || [ ! -f "$expected_exitcode" ]; then
        test_passed=false
        differences="Expected files not found"
    else
        local expected_exit=$(cat "$expected_exitcode")
        local actual_exit=$(cat "$actual_exitcode")
        if [ "$expected_exit" != "$actual_exit" ]; then
            test_passed=false
            differences="${differences}Exit code: expected=$expected_exit, actual=$actual_exit; "
        fi
        if ! diff -q "$expected_stdout" "$actual_stdout" > /dev/null 2>&1; then
            test_passed=false
            differences="${differences}Stdout differs; "
        fi
        if ! diff -q "$expected_stderr" "$actual_stderr" > /dev/null 2>&1; then
            test_passed=false
            differences="${differences}Stderr differs; "
        fi
    fi

    {
        echo "=== Test ${test_num} (${test_name}) ==="
        echo "Command: ./${binary} ${args[*]}"
        echo "--- stdout ---"
        cat "$actual_stdout" 2>/dev/null
        echo "--- stderr ---"
        cat "$actual_stderr" 2>/dev/null
        echo "--- exitcode ---"
        cat "$actual_exitcode" 2>/dev/null
        echo "--- differences ---"
        echo -e "$differences"
    } > "$log_tmp"

    if [ "$test_passed" = true ]; then
        mv "$log_tmp" "flow_results/test${test_num}_success.log"
        echo "Test ${test_num} passed"
    else
        mv "$log_tmp" "flow_results/test${test_num}_fail.log"
        echo "Test ${test_num} failed" >&2
        failed=1
    fi

    echo "Test ${test_num} ended"
}

run_case 1  "test01_no_args"           "su-exec_t1"
run_case 2  "test02_one_arg"           "su-exec_t2"  user1
run_case 3  "test03_help_flag"         "su-exec_t3"  --help
run_case 4  "test04_invalid_option"    "su-exec_t4"  -h
run_case 5  "test05_basic_root"        "su-exec_t5"  root echo "Hello World"
run_case 6  "test06_user_group"        "su-exec_t6"  user:group echo test
run_case 7  "test07_numeric_uid"       "su-exec_t7"  1000 echo test
run_case 8  "test08_numeric_gid"       "su-exec_t8"  user:1000 echo test
run_case 9  "test09_nonexistent_user"  "su-exec_t9"  nonexistent_user echo test
run_case 10 "test10_nonexistent_group" "su-exec_t10" user:nonexistent_group echo test
run_case 11 "test11_valid_user_group"  "su-exec_t11" root:root echo test
run_case 12 "test12_numeric_gid_root"  "su-exec_t12" root:0 echo test
run_case 13 "test13_empty_username"    "su-exec_t13" "" echo test
run_case 14 "test14_numeric_uid_only"  "su-exec_t14" 0 echo test
run_case 15 "test15_large_numeric_uid" "su-exec_t15" 65534 echo test
run_case 16 "test16_invalid_large_uid" "su-exec_t16" 4294967296 echo test
run_case 17 "test17_negative_uid"      "su-exec_t17" -1 echo test
run_case 18 "test18_user_colon_no_group" "su-exec_t18" "user:" echo test
run_case 19 "test19_only_colon"        "su-exec_t19" ":" echo test
run_case 20 "test20_multiple_colons"   "su-exec_t20" user:group:extra echo test

exit $failed

