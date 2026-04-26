
#!/bin/bash

# Reformed test cases

failed=0

EXPECTED_DIR="expected"
ACTUAL_DIR="actual"
RESULTS_DIR="results"
FLOW_DIR="flow_results"

mkdir -p "$ACTUAL_DIR" "$RESULTS_DIR" "$FLOW_DIR"

# Normalize differences that come from using unique per-testcase binary names.
# The expected outputs were captured with the binary './su-exec', but we run
# './test_su_exec_tN'. So the program name embedded in stdout/stderr (argv[0]
# and program_invocation_short_name) will differ. Normalize both sides.
#
# Also ignore spurious "[tracer] failed to open trace file" lines emitted by
# the LD_PRELOAD tracer in some testcases (when the target exec's into another
# program that does not have the tracer configured).
normalize() {
    local in_file="$1"
    local bin_name="$2"
    local out_file="$3"
    if [ ! -f "$in_file" ]; then
        : > "$out_file"
        return
    fi
    sed -e "s#\./${bin_name}#./su-exec#g" \
        -e "s#\b${bin_name}:#su-exec:#g" \
        -e '/^\[tracer\] failed to open trace file$/d' \
        "$in_file" > "$out_file"
}

# run_case <test_num> <test_name> <binary> <args...>
run_case() {
    local test_num="$1"
    local test_name="$2"
    local binary="$3"
    shift 3
    local args=("$@")

    echo "Test ${test_num} started"

    local actual_stdout="$ACTUAL_DIR/${test_name}_stdout.txt"
    local actual_stderr="$ACTUAL_DIR/${test_name}_stderr.txt"
    local actual_exitcode="$ACTUAL_DIR/${test_name}_exitcode.txt"

    local expected_stdout="$EXPECTED_DIR/${test_name}_stdout.txt"
    local expected_stderr="$EXPECTED_DIR/${test_name}_stderr.txt"
    local expected_exitcode="$EXPECTED_DIR/${test_name}_exitcode.txt"

    local success_log="$FLOW_DIR/test${test_num}_success.log"
    local fail_log="$FLOW_DIR/test${test_num}_fail.log"
    local trace_log="$PWD/$FLOW_DIR/test${test_num}_trace.log"
    local tmp_log="$FLOW_DIR/test${test_num}_tmp.log"

    : > "$tmp_log"

    # Remove stale success/fail logs from previous runs so we don't leave both.
    rm -f "$success_log" "$fail_log"

    if [ ! -x "./$binary" ]; then
        echo "Binary ./$binary not found or not executable" >> "$tmp_log"
        echo "Test ${test_num} failed" >&2
        mv "$tmp_log" "$fail_log"
        echo "Test ${test_num} failed"
        echo "Test ${test_num} ended"
        failed=1
        return
    fi

    echo "Running: $test_name with binary ./$binary args: ${args[*]}" >> "$tmp_log"

    LD_PRELOAD=libtracer.so TRACE_OUTPUT="$trace_log" "./$binary" "${args[@]}" > "$actual_stdout" 2> "$actual_stderr"
    local rc=$?
    echo "$rc" > "$actual_exitcode"

    # Build normalized copies for comparison
    local norm_actual_stdout="$ACTUAL_DIR/${test_name}_stdout.norm.txt"
    local norm_actual_stderr="$ACTUAL_DIR/${test_name}_stderr.norm.txt"
    local norm_expected_stdout="$ACTUAL_DIR/${test_name}_stdout.expected.norm.txt"
    local norm_expected_stderr="$ACTUAL_DIR/${test_name}_stderr.expected.norm.txt"

    normalize "$actual_stdout" "$binary" "$norm_actual_stdout"
    normalize "$actual_stderr" "$binary" "$norm_actual_stderr"
    normalize "$expected_stdout" "su-exec" "$norm_expected_stdout"
    normalize "$expected_stderr" "su-exec" "$norm_expected_stderr"

    local test_passed=true

    if [ ! -f "$expected_stdout" ] || [ ! -f "$expected_stderr" ] || [ ! -f "$expected_exitcode" ]; then
        echo "Expected files not found for $test_name" >> "$tmp_log"
        test_passed=false
    else
        local expected_exit
        local actual_exit
        expected_exit=$(cat "$expected_exitcode")
        actual_exit=$(cat "$actual_exitcode")

        if [ "$expected_exit" != "$actual_exit" ]; then
            test_passed=false
            echo "Exit code differs: expected=$expected_exit, actual=$actual_exit" >> "$tmp_log"
        fi

        if ! diff -q "$norm_expected_stdout" "$norm_actual_stdout" > /dev/null 2>&1; then
            test_passed=false
            echo "Stdout differs (after normalization):" >> "$tmp_log"
            diff "$norm_expected_stdout" "$norm_actual_stdout" >> "$tmp_log" 2>&1
        fi

        if ! diff -q "$norm_expected_stderr" "$norm_actual_stderr" > /dev/null 2>&1; then
            test_passed=false
            echo "Stderr differs (after normalization):" >> "$tmp_log"
            diff "$norm_expected_stderr" "$norm_actual_stderr" >> "$tmp_log" 2>&1
        fi
    fi

    if [ "$test_passed" = true ]; then
        mv "$tmp_log" "$success_log"
        echo "Test ${test_num} passed"
    else
        mv "$tmp_log" "$fail_log"
        echo "Test ${test_num} failed" >&2
        echo "Test ${test_num} failed"
        failed=1
    fi

    echo "Test ${test_num} ended"
}

run_case 1  "test01_no_args"            "test_su_exec_t1"
run_case 2  "test02_one_arg"            "test_su_exec_t2"  "user1"
run_case 3  "test03_help_flag"          "test_su_exec_t3"  "--help"
run_case 4  "test04_invalid_option"     "test_su_exec_t4"  "-h"
run_case 5  "test05_basic_root"         "test_su_exec_t5"  "root" "echo" "Hello World"
run_case 6  "test06_user_group"         "test_su_exec_t6"  "user:group" "echo" "test"
run_case 7  "test07_numeric_uid"        "test_su_exec_t7"  "1000" "echo" "test"
run_case 8  "test08_numeric_gid"        "test_su_exec_t8"  "user:1000" "echo" "test"
run_case 9  "test09_nonexistent_user"   "test_su_exec_t9"  "nonexistent_user" "echo" "test"
run_case 10 "test10_nonexistent_group"  "test_su_exec_t10" "user:nonexistent_group" "echo" "test"
run_case 11 "test11_valid_user_group"   "test_su_exec_t11" "root:root" "echo" "test"
run_case 12 "test12_numeric_gid_root"   "test_su_exec_t12" "root:0" "echo" "test"
run_case 13 "test13_empty_username"     "test_su_exec_t13" "" "echo" "test"
run_case 14 "test14_numeric_uid_only"   "test_su_exec_t14" "0" "echo" "test"
run_case 15 "test15_large_numeric_uid"  "test_su_exec_t15" "65534" "echo" "test"
run_case 16 "test16_invalid_large_uid"  "test_su_exec_t16" "4294967296" "echo" "test"
run_case 17 "test17_negative_uid"       "test_su_exec_t17" "-1" "echo" "test"
run_case 18 "test18_user_colon_no_group" "test_su_exec_t18" "user:" "echo" "test"
run_case 19 "test19_only_colon"         "test_su_exec_t19" ":" "echo" "test"
run_case 20 "test20_multiple_colons"    "test_su_exec_t20" "user:group:extra" "echo" "test"

exit $failed

