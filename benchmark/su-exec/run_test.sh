
#!/bin/bash

# Reformed test cases for su-exec

failed=0

RESULTS_DIR="flow_results"
ACTUAL_DIR="actual"

# Create directories
mkdir -p "$RESULTS_DIR" "$ACTUAL_DIR"

# Determine if we are running as root
CURRENT_UID=$(id -u)

###############################################################################
# Test 1: Execute su-exec without any arguments (expects usage on stdout, exit 1)
###############################################################################
echo "Test 1 started"
test1_log=""
test1_pass=true

LD_PRELOAD=libtracer.so TRACE_OUTPUT=$PWD/flow_results/test1_trace.log ./su-exec_t1 > "$ACTUAL_DIR/t1_stdout.txt" 2> "$ACTUAL_DIR/t1_stderr.txt"
test1_exit=$?

# Expect exit code 1 (usage error)
if [ "$test1_exit" -ne 1 ]; then
    test1_pass=false
    test1_log="${test1_log}Exit code: expected=1, actual=$test1_exit\n"
fi

# Usage message goes to stdout (printf in usage())
if ! grep -q "Usage:" "$ACTUAL_DIR/t1_stdout.txt"; then
    test1_pass=false
    test1_log="${test1_log}Stdout does not contain 'Usage:'\n"
fi

if [ "$test1_pass" = true ]; then
    echo "Test 1 passed"
    echo "Test 1 passed" > "$RESULTS_DIR/test1_success.log"
else
    echo "Test 1 failed"
    echo "Test 1 failed" >&2
    echo -e "$test1_log" > "$RESULTS_DIR/test1_fail.log"
    failed=1
fi
echo "Test 1 ended"

###############################################################################
# Test 2: Execute su-exec with only one argument (user but no command)
###############################################################################
echo "Test 2 started"
test2_log=""
test2_pass=true

LD_PRELOAD=libtracer.so TRACE_OUTPUT=$PWD/flow_results/test2_trace.log ./su-exec_t2 root > "$ACTUAL_DIR/t2_stdout.txt" 2> "$ACTUAL_DIR/t2_stderr.txt"
test2_exit=$?

# Expect exit code 1 (usage error - argc < 3)
if [ "$test2_exit" -ne 1 ]; then
    test2_pass=false
    test2_log="${test2_log}Exit code: expected=1, actual=$test2_exit\n"
fi

# Usage message goes to stdout
if ! grep -q "Usage:" "$ACTUAL_DIR/t2_stdout.txt"; then
    test2_pass=false
    test2_log="${test2_log}Stdout does not contain 'Usage:'\n"
fi

if [ "$test2_pass" = true ]; then
    echo "Test 2 passed"
    echo "Test 2 passed" > "$RESULTS_DIR/test2_success.log"
else
    echo "Test 2 failed"
    echo "Test 2 failed" >&2
    echo -e "$test2_log" > "$RESULTS_DIR/test2_fail.log"
    failed=1
fi
echo "Test 2 ended"

###############################################################################
# Test 3: Execute su-exec with --help flag (treated as user, argc < 3 => usage)
###############################################################################
echo "Test 3 started"
test3_log=""
test3_pass=true

LD_PRELOAD=libtracer.so TRACE_OUTPUT=$PWD/flow_results/test3_trace.log ./su-exec_t3 --help > "$ACTUAL_DIR/t3_stdout.txt" 2> "$ACTUAL_DIR/t3_stderr.txt"
test3_exit=$?

# --help is treated as a username with no command, argc < 3 => usage
if [ "$test3_exit" -ne 1 ]; then
    test3_pass=false
    test3_log="${test3_log}Exit code: expected=1, actual=$test3_exit\n"
fi

# Usage message goes to stdout
if ! grep -q "Usage:" "$ACTUAL_DIR/t3_stdout.txt"; then
    test3_pass=false
    test3_log="${test3_log}Stdout does not contain 'Usage:'\n"
fi

if [ "$test3_pass" = true ]; then
    echo "Test 3 passed"
    echo "Test 3 passed" > "$RESULTS_DIR/test3_success.log"
else
    echo "Test 3 failed"
    echo "Test 3 failed" >&2
    echo -e "$test3_log" > "$RESULTS_DIR/test3_fail.log"
    failed=1
fi
echo "Test 3 ended"

###############################################################################
# Test 4: Execute su-exec with -h flag (treated as user, argc < 3 => usage)
###############################################################################
echo "Test 4 started"
test4_log=""
test4_pass=true

LD_PRELOAD=libtracer.so TRACE_OUTPUT=$PWD/flow_results/test4_trace.log ./su-exec_t4 -h > "$ACTUAL_DIR/t4_stdout.txt" 2> "$ACTUAL_DIR/t4_stderr.txt"
test4_exit=$?

if [ "$test4_exit" -ne 1 ]; then
    test4_pass=false
    test4_log="${test4_log}Exit code: expected=1, actual=$test4_exit\n"
fi

# Usage message goes to stdout
if ! grep -q "Usage:" "$ACTUAL_DIR/t4_stdout.txt"; then
    test4_pass=false
    test4_log="${test4_log}Stdout does not contain 'Usage:'\n"
fi

if [ "$test4_pass" = true ]; then
    echo "Test 4 passed"
    echo "Test 4 passed" > "$RESULTS_DIR/test4_success.log"
else
    echo "Test 4 failed"
    echo "Test 4 failed" >&2
    echo -e "$test4_log" > "$RESULTS_DIR/test4_fail.log"
    failed=1
fi
echo "Test 4 ended"

###############################################################################
# Test 5: Execute su-exec as root running echo
# When not root: setgroups fails => exit 1 with error on stderr
# When root: echo succeeds => exit 0 with output on stdout
###############################################################################
echo "Test 5 started"
test5_log=""
test5_pass=true

LD_PRELOAD=libtracer.so TRACE_OUTPUT=$PWD/flow_results/test5_trace.log ./su-exec_t5 root echo "Hello World" > "$ACTUAL_DIR/t5_stdout.txt" 2> "$ACTUAL_DIR/t5_stderr.txt"
test5_exit=$?

if [ "$CURRENT_UID" -eq 0 ]; then
    if [ "$test5_exit" -ne 0 ]; then
        test5_pass=false
        test5_log="${test5_log}Exit code: expected=0, actual=$test5_exit\n"
    fi
    actual_stdout=$(cat "$ACTUAL_DIR/t5_stdout.txt")
    if [ "$actual_stdout" != "Hello World" ]; then
        test5_pass=false
        test5_log="${test5_log}Stdout: expected='Hello World', actual='$actual_stdout'\n"
    fi
else
    # Not root: setgroups will fail
    if [ "$test5_exit" -ne 1 ]; then
        test5_pass=false
        test5_log="${test5_log}Exit code: expected=1 (non-root), actual=$test5_exit\n"
    fi
    if ! grep -q "setgroups" "$ACTUAL_DIR/t5_stderr.txt"; then
        test5_pass=false
        test5_log="${test5_log}Stderr does not contain 'setgroups' error\n"
    fi
fi

if [ "$test5_pass" = true ]; then
    echo "Test 5 passed"
    echo "Test 5 passed" > "$RESULTS_DIR/test5_success.log"
else
    echo "Test 5 failed"
    echo "Test 5 failed" >&2
    echo -e "$test5_log" > "$RESULTS_DIR/test5_fail.log"
    failed=1
fi
echo "Test 5 ended"

###############################################################################
# Test 6: Test with nobody:nogroup specification
# When not root: setgroups fails => exit 1 with error on stderr
###############################################################################
echo "Test 6 started"
test6_log=""
test6_pass=true

LD_PRELOAD=libtracer.so TRACE_OUTPUT=$PWD/flow_results/test6_trace.log ./su-exec_t6 nobody:nogroup echo "test" > "$ACTUAL_DIR/t6_stdout.txt" 2> "$ACTUAL_DIR/t6_stderr.txt"
test6_exit=$?

if [ "$CURRENT_UID" -eq 0 ]; then
    # As root, this should work if nobody and nogroup exist
    if [ "$test6_exit" -eq 0 ]; then
        actual_stdout=$(cat "$ACTUAL_DIR/t6_stdout.txt")
        if [ "$actual_stdout" != "test" ]; then
            test6_pass=false
            test6_log="${test6_log}Stdout: expected='test', actual='$actual_stdout'\n"
        fi
    else
        # May fail if nogroup doesn't exist
        if [ ! -s "$ACTUAL_DIR/t6_stderr.txt" ]; then
            test6_pass=false
            test6_log="${test6_log}Non-zero exit ($test6_exit) but no stderr output\n"
        fi
    fi
else
    # Not root: expect failure due to setgroups or getgrnam
    if [ "$test6_exit" -eq 0 ]; then
        test6_pass=false
        test6_log="${test6_log}Exit code: expected non-zero (non-root), actual=0\n"
    fi
    # Stderr should have an error message
    if [ ! -s "$ACTUAL_DIR/t6_stderr.txt" ]; then
        test6_pass=false
        test6_log="${test6_log}Stderr is empty, expected error message\n"
    fi
fi

if [ "$test6_pass" = true ]; then
    echo "Test 6 passed"
    echo "Test 6 passed" > "$RESULTS_DIR/test6_success.log"
else
    echo "Test 6 failed"
    echo "Test 6 failed" >&2
    echo -e "$test6_log" > "$RESULTS_DIR/test6_fail.log"
    failed=1
fi
echo "Test 6 ended"

###############################################################################
# Test 7: Test with numeric UID 0 running id -u
# When not root: setgroups fails
###############################################################################
echo "Test 7 started"
test7_log=""
test7_pass=true

LD_PRELOAD=libtracer.so TRACE_OUTPUT=$PWD/flow_results/test7_trace.log ./su-exec_t7 0 id -u > "$ACTUAL_DIR/t7_stdout.txt" 2> "$ACTUAL_DIR/t7_stderr.txt"
test7_exit=$?

if [ "$CURRENT_UID" -eq 0 ]; then
    if [ "$test7_exit" -ne 0 ]; then
        test7_pass=false
        test7_log="${test7_log}Exit code: expected=0, actual=$test7_exit\n"
    fi
    actual_stdout=$(cat "$ACTUAL_DIR/t7_stdout.txt" | tr -d '\n')
    if [ "$actual_stdout" != "0" ]; then
        test7_pass=false
        test7_log="${test7_log}Stdout: expected='0', actual='$actual_stdout'\n"
    fi
else
    # Not root: setgroups will fail
    if [ "$test7_exit" -ne 1 ]; then
        test7_pass=false
        test7_log="${test7_log}Exit code: expected=1 (non-root), actual=$test7_exit\n"
    fi
    if ! grep -q "setgroups" "$ACTUAL_DIR/t7_stderr.txt"; then
        test7_pass=false
        test7_log="${test7_log}Stderr does not contain 'setgroups' error\n"
    fi
fi

if [ "$test7_pass" = true ]; then
    echo "Test 7 passed"
    echo "Test 7 passed" > "$RESULTS_DIR/test7_success.log"
else
    echo "Test 7 failed"
    echo "Test 7 failed" >&2
    echo -e "$test7_log" > "$RESULTS_DIR/test7_fail.log"
    failed=1
fi
echo "Test 7 ended"

###############################################################################
# Test 8: Test with numeric UID:GID 0:0 running id -g
# When not root: setgroups fails
###############################################################################
echo "Test 8 started"
test8_log=""
test8_pass=true

LD_PRELOAD=libtracer.so TRACE_OUTPUT=$PWD/flow_results/test8_trace.log ./su-exec_t8 0:0 id -g > "$ACTUAL_DIR/t8_stdout.txt" 2> "$ACTUAL_DIR/t8_stderr.txt"
test8_exit=$?

if [ "$CURRENT_UID" -eq 0 ]; then
    if [ "$test8_exit" -ne 0 ]; then
        test8_pass=false
        test8_log="${test8_log}Exit code: expected=0, actual=$test8_exit\n"
    fi
    actual_stdout=$(cat "$ACTUAL_DIR/t8_stdout.txt" | tr -d '\n')
    if [ "$actual_stdout" != "0" ]; then
        test8_pass=false
        test8_log="${test8_log}Stdout: expected='0', actual='$actual_stdout'\n"
    fi
else
    # Not root: setgroups will fail
    if [ "$test8_exit" -ne 1 ]; then
        test8_pass=false
        test8_log="${test8_log}Exit code: expected=1 (non-root), actual=$test8_exit\n"
    fi
    if ! grep -q "setgroups" "$ACTUAL_DIR/t8_stderr.txt"; then
        test8_pass=false
        test8_log="${test8_log}Stderr does not contain 'setgroups' error\n"
    fi
fi

if [ "$test8_pass" = true ]; then
    echo "Test 8 passed"
    echo "Test 8 passed" > "$RESULTS_DIR/test8_success.log"
else
    echo "Test 8 failed"
    echo "Test 8 failed" >&2
    echo -e "$test8_log" > "$RESULTS_DIR/test8_fail.log"
    failed=1
fi
echo "Test 8 ended"

###############################################################################
# Test 9: Test with non-existent user
###############################################################################
echo "Test 9 started"
test9_log=""
test9_pass=true

LD_PRELOAD=libtracer.so TRACE_OUTPUT=$PWD/flow_results/test9_trace.log ./su-exec_t9 nonexistent_user_xyz echo "test" > "$ACTUAL_DIR/t9_stdout.txt" 2> "$ACTUAL_DIR/t9_stderr.txt"
test9_exit=$?

# Non-existent user should cause getpwnam error (exit 1)
if [ "$test9_exit" -ne 1 ]; then
    test9_pass=false
    test9_log="${test9_log}Exit code: expected=1, actual=$test9_exit\n"
fi

# Stderr should contain getpwnam error
if ! grep -q "getpwnam" "$ACTUAL_DIR/t9_stderr.txt"; then
    test9_pass=false
    test9_log="${test9_log}Stderr does not contain 'getpwnam' error\n"
fi

if [ "$test9_pass" = true ]; then
    echo "Test 9 passed"
    echo "Test 9 passed" > "$RESULTS_DIR/test9_success.log"
else
    echo "Test 9 failed"
    echo "Test 9 failed" >&2
    echo -e "$test9_log" > "$RESULTS_DIR/test9_fail.log"
    failed=1
fi
echo "Test 9 ended"

###############################################################################
# Test 10: Test with non-existent group
###############################################################################
echo "Test 10 started"
test10_log=""
test10_pass=true

LD_PRELOAD=libtracer.so TRACE_OUTPUT=$PWD/flow_results/test10_trace.log ./su-exec_t10 root:nonexistent_group_xyz echo "test" > "$ACTUAL_DIR/t10_stdout.txt" 2> "$ACTUAL_DIR/t10_stderr.txt"
test10_exit=$?

# Non-existent group should cause getgrnam error (exit 1)
if [ "$test10_exit" -ne 1 ]; then
    test10_pass=false
    test10_log="${test10_log}Exit code: expected=1, actual=$test10_exit\n"
fi

# Stderr should contain getgrnam error
if ! grep -q "getgrnam" "$ACTUAL_DIR/t10_stderr.txt"; then
    test10_pass=false
    test10_log="${test10_log}Stderr does not contain 'getgrnam' error\n"
fi

if [ "$test10_pass" = true ]; then
    echo "Test 10 passed"
    echo "Test 10 passed" > "$RESULTS_DIR/test10_success.log"
else
    echo "Test 10 failed"
    echo "Test 10 failed" >&2
    echo -e "$test10_log" > "$RESULTS_DIR/test10_fail.log"
    failed=1
fi
echo "Test 10 ended"

###############################################################################
# Test 11: Test with valid user:group combination (root:root)
# When not root: setgroups(0) fails
###############################################################################
echo "Test 11 started"
test11_log=""
test11_pass=true

LD_PRELOAD=libtracer.so TRACE_OUTPUT=$PWD/flow_results/test11_trace.log ./su-exec_t11 root:root echo "test" > "$ACTUAL_DIR/t11_stdout.txt" 2> "$ACTUAL_DIR/t11_stderr.txt"
test11_exit=$?

if [ "$CURRENT_UID" -eq 0 ]; then
    if [ "$test11_exit" -ne 0 ]; then
        test11_pass=false
        test11_log="${test11_log}Exit code: expected=0, actual=$test11_exit\n"
    fi
    actual_stdout=$(cat "$ACTUAL_DIR/t11_stdout.txt")
    if [ "$actual_stdout" != "test" ]; then
        test11_pass=false
        test11_log="${test11_log}Stdout: expected='test', actual='$actual_stdout'\n"
    fi
else
    # Not root: setgroups will fail
    if [ "$test11_exit" -ne 1 ]; then
        test11_pass=false
        test11_log="${test11_log}Exit code: expected=1 (non-root), actual=$test11_exit\n"
    fi
    if ! grep -q "setgroups" "$ACTUAL_DIR/t11_stderr.txt"; then
        test11_pass=false
        test11_log="${test11_log}Stderr does not contain 'setgroups' error\n"
    fi
fi

if [ "$test11_pass" = true ]; then
    echo "Test 11 passed"
    echo "Test 11 passed" > "$RESULTS_DIR/test11_success.log"
else
    echo "Test 11 failed"
    echo "Test 11 failed" >&2
    echo -e "$test11_log" > "$RESULTS_DIR/test11_fail.log"
    failed=1
fi
echo "Test 11 ended"

###############################################################################
# Test 12: Test with root:0 (numeric GID)
# When not root: setgroups fails
###############################################################################
echo "Test 12 started"
test12_log=""
test12_pass=true

LD_PRELOAD=libtracer.so TRACE_OUTPUT=$PWD/flow_results/test12_trace.log ./su-exec_t12 root:0 id -g > "$ACTUAL_DIR/t12_stdout.txt" 2> "$ACTUAL_DIR/t12_stderr.txt"
test12_exit=$?

if [ "$CURRENT_UID" -eq 0 ]; then
    if [ "$test12_exit" -ne 0 ]; then
        test12_pass=false
        test12_log="${test12_log}Exit code: expected=0, actual=$test12_exit\n"
    fi
    actual_stdout=$(cat "$ACTUAL_DIR/t12_stdout.txt" | tr -d '\n')
    if [ "$actual_stdout" != "0" ]; then
        test12_pass=false
        test12_log="${test12_log}Stdout: expected='0', actual='$actual_stdout'\n"
    fi
else
    # Not root: setgroups will fail
    if [ "$test12_exit" -ne 1 ]; then
        test12_pass=false
        test12_log="${test12_log}Exit code: expected=1 (non-root), actual=$test12_exit\n"
    fi
    if ! grep -q "setgroups" "$ACTUAL_DIR/t12_stderr.txt"; then
        test12_pass=false
        test12_log="${test12_log}Stderr does not contain 'setgroups' error\n"
    fi
fi

if [ "$test12_pass" = true ]; then
    echo "Test 12 passed"
    echo "Test 12 passed" > "$RESULTS_DIR/test12_success.log"
else
    echo "Test 12 failed"
    echo "Test 12 failed" >&2
    echo -e "$test12_log" > "$RESULTS_DIR/test12_fail.log"
    failed=1
fi
echo "Test 12 ended"

###############################################################################
# Test 13: Test with empty username
# Empty user => user[0] == '\0', so uid stays as getuid(), pw stays NULL
# Then pw=NULL => setgroups(1, &gid) which fails without root
###############################################################################
echo "Test 13 started"
test13_log=""
test13_pass=true

LD_PRELOAD=libtracer.so TRACE_OUTPUT=$PWD/flow_results/test13_trace.log ./su-exec_t13 '' echo "test" > "$ACTUAL_DIR/t13_stdout.txt" 2> "$ACTUAL_DIR/t13_stderr.txt"
test13_exit=$?

if [ "$CURRENT_UID" -eq 0 ]; then
    # As root with empty user, uid=0, should work
    if [ "$test13_exit" -ne 0 ]; then
        test13_pass=false
        test13_log="${test13_log}Exit code: expected=0, actual=$test13_exit\n"
    fi
else
    # Not root: setgroups will fail
    if [ "$test13_exit" -ne 1 ]; then
        test13_pass=false
        test13_log="${test13_log}Exit code: expected=1 (non-root), actual=$test13_exit\n"
    fi
    if [ ! -s "$ACTUAL_DIR/t13_stderr.txt" ]; then
        test13_pass=false
        test13_log="${test13_log}Stderr is empty, expected error message\n"
    fi
fi

if [ "$test13_pass" = true ]; then
    echo "Test 13 passed"
    echo "Test 13 passed" > "$RESULTS_DIR/test13_success.log"
else
    echo "Test 13 failed"
    echo "Test 13 failed" >&2
    echo -e "$test13_log" > "$RESULTS_DIR/test13_fail.log"
    failed=1
fi
echo "Test 13 ended"

###############################################################################
# Test 14: Test with numeric UID 0 running id -u
# When not root: setgroups fails
###############################################################################
echo "Test 14 started"
test14_log=""
test14_pass=true

LD_PRELOAD=libtracer.so TRACE_OUTPUT=$PWD/flow_results/test14_trace.log ./su-exec_t14 0 id -u > "$ACTUAL_DIR/t14_stdout.txt" 2> "$ACTUAL_DIR/t14_stderr.txt"
test14_exit=$?

if [ "$CURRENT_UID" -eq 0 ]; then
    if [ "$test14_exit" -ne 0 ]; then
        test14_pass=false
        test14_log="${test14_log}Exit code: expected=0, actual=$test14_exit\n"
    fi
    actual_stdout=$(cat "$ACTUAL_DIR/t14_stdout.txt" | tr -d '\n')
    if [ "$actual_stdout" != "0" ]; then
        test14_pass=false
        test14_log="${test14_log}Stdout: expected='0', actual='$actual_stdout'\n"
    fi
else
    # Not root: setgroups will fail
    if [ "$test14_exit" -ne 1 ]; then
        test14_pass=false
        test14_log="${test14_log}Exit code: expected=1 (non-root), actual=$test14_exit\n"
    fi
    if ! grep -q "setgroups" "$ACTUAL_DIR/t14_stderr.txt"; then
        test14_pass=false
        test14_log="${test14_log}Stderr does not contain 'setgroups' error\n"
    fi
fi

if [ "$test14_pass" = true ]; then
    echo "Test 14 passed"
    echo "Test 14 passed" > "$RESULTS_DIR/test14_success.log"
else
    echo "Test 14 failed"
    echo "Test 14 failed" >&2
    echo -e "$test14_log" > "$RESULTS_DIR/test14_fail.log"
    failed=1
fi
echo "Test 14 ended"

###############################################################################
# Test 15: Test with large numeric UID (65534 = nobody)
# When not root: setgroups fails
###############################################################################
echo "Test 15 started"
test15_log=""
test15_pass=true

LD_PRELOAD=libtracer.so TRACE_OUTPUT=$PWD/flow_results/test15_trace.log ./su-exec_t15 65534 id -u > "$ACTUAL_DIR/t15_stdout.txt" 2> "$ACTUAL_DIR/t15_stderr.txt"
test15_exit=$?

if [ "$CURRENT_UID" -eq 0 ]; then
    if [ "$test15_exit" -ne 0 ]; then
        test15_pass=false
        test15_log="${test15_log}Exit code: expected=0, actual=$test15_exit\n"
    fi
    actual_stdout=$(cat "$ACTUAL_DIR/t15_stdout.txt" | tr -d '\n')
    if [ "$actual_stdout" != "65534" ]; then
        test15_pass=false
        test15_log="${test15_log}Stdout: expected='65534', actual='$actual_stdout'\n"
    fi
else
    # Not root: setgroups will fail
    if [ "$test15_exit" -ne 1 ]; then
        test15_pass=false
        test15_log="${test15_log}Exit code: expected=1 (non-root), actual=$test15_exit\n"
    fi
    if ! grep -q "setgroups" "$ACTUAL_DIR/t15_stderr.txt"; then
        test15_pass=false
        test15_log="${test15_log}Stderr does not contain 'setgroups' error\n"
    fi
fi

if [ "$test15_pass" = true ]; then
    echo "Test 15 passed"
    echo "Test 15 passed" > "$RESULTS_DIR/test15_success.log"
else
    echo "Test 15 failed"
    echo "Test 15 failed" >&2
    echo -e "$test15_log" > "$RESULTS_DIR/test15_fail.log"
    failed=1
fi
echo "Test 15 ended"

###############################################################################
# Test 16: Test with invalid command (nonexistent binary)
# When not root: setgroups fails before reaching execvp
###############################################################################
echo "Test 16 started"
test16_log=""
test16_pass=true

LD_PRELOAD=libtracer.so TRACE_OUTPUT=$PWD/flow_results/test16_trace.log ./su-exec_t16 root /nonexistent_binary_xyz > "$ACTUAL_DIR/t16_stdout.txt" 2> "$ACTUAL_DIR/t16_stderr.txt"
test16_exit=$?

# Whether root or not, this should fail (exit 1)
# As root: execvp fails; not root: setgroups fails
if [ "$test16_exit" -ne 1 ]; then
    test16_pass=false
    test16_log="${test16_log}Exit code: expected=1, actual=$test16_exit\n"
fi

# Stderr should contain an error message
if [ ! -s "$ACTUAL_DIR/t16_stderr.txt" ]; then
    test16_pass=false
    test16_log="${test16_log}Stderr is empty, expected error message\n"
fi

if [ "$test16_pass" = true ]; then
    echo "Test 16 passed"
    echo "Test 16 passed" > "$RESULTS_DIR/test16_success.log"
else
    echo "Test 16 failed"
    echo "Test 16 failed" >&2
    echo -e "$test16_log" > "$RESULTS_DIR/test16_fail.log"
    failed=1
fi
echo "Test 16 ended"

###############################################################################
# Test 17: Test root running id -u
# When not root: setgroups fails
###############################################################################
echo "Test 17 started"
test17_log=""
test17_pass=true

LD_PRELOAD=libtracer.so TRACE_OUTPUT=$PWD/flow_results/test17_trace.log ./su-exec_t17 root id -u > "$ACTUAL_DIR/t17_stdout.txt" 2> "$ACTUAL_DIR/t17_stderr.txt"
test17_exit=$?

if [ "$CURRENT_UID" -eq 0 ]; then
    if [ "$test17_exit" -ne 0 ]; then
        test17_pass=false
        test17_log="${test17_log}Exit code: expected=0, actual=$test17_exit\n"
    fi
    actual_stdout=$(cat "$ACTUAL_DIR/t17_stdout.txt" | tr -d '\n')
    if [ "$actual_stdout" != "0" ]; then
        test17_pass=false
        test17_log="${test17_log}Stdout: expected='0', actual='$actual_stdout'\n"
    fi
else
    # Not root: setgroups will fail
    if [ "$test17_exit" -ne 1 ]; then
        test17_pass=false
        test17_log="${test17_log}Exit code: expected=1 (non-root), actual=$test17_exit\n"
    fi
    if ! grep -q "setgroups" "$ACTUAL_DIR/t17_stderr.txt"; then
        test17_pass=false
        test17_log="${test17_log}Stderr does not contain 'setgroups' error\n"
    fi
fi

if [ "$test17_pass" = true ]; then
    echo "Test 17 passed"
    echo "Test 17 passed" > "$RESULTS_DIR/test17_success.log"
else
    echo "Test 17 failed"
    echo "Test 17 failed" >&2
    echo -e "$test17_log" > "$RESULTS_DIR/test17_fail.log"
    failed=1
fi
echo "Test 17 ended"

###############################################################################
# Test 18: Test root running id -g
# When not root: setgroups fails
###############################################################################
echo "Test 18 started"
test18_log=""
test18_pass=true

LD_PRELOAD=libtracer.so TRACE_OUTPUT=$PWD/flow_results/test18_trace.log ./su-exec_t18 root id -g > "$ACTUAL_DIR/t18_stdout.txt" 2> "$ACTUAL_DIR/t18_stderr.txt"
test18_exit=$?

if [ "$CURRENT_UID" -eq 0 ]; then
    if [ "$test18_exit" -ne 0 ]; then
        test18_pass=false
        test18_log="${test18_log}Exit code: expected=0, actual=$test18_exit\n"
    fi
    actual_stdout=$(cat "$ACTUAL_DIR/t18_stdout.txt" | tr -d '\n')
    if [ "$actual_stdout" != "0" ]; then
        test18_pass=false
        test18_log="${test18_log}Stdout: expected='0', actual='$actual_stdout'\n"
    fi
else
    # Not root: setgroups will fail
    if [ "$test18_exit" -ne 1 ]; then
        test18_pass=false
        test18_log="${test18_log}Exit code: expected=1 (non-root), actual=$test18_exit\n"
    fi
    if ! grep -q "setgroups" "$ACTUAL_DIR/t18_stderr.txt"; then
        test18_pass=false
        test18_log="${test18_log}Stderr does not contain 'setgroups' error\n"
    fi
fi

if [ "$test18_pass" = true ]; then
    echo "Test 18 passed"
    echo "Test 18 passed" > "$RESULTS_DIR/test18_success.log"
else
    echo "Test 18 failed"
    echo "Test 18 failed" >&2
    echo -e "$test18_log" > "$RESULTS_DIR/test18_fail.log"
    failed=1
fi
echo "Test 18 ended"

###############################################################################
# Test 19: Test with colon only (empty user and empty group)
# ':' => user="", group="" (empty after split)
# user[0]=='\0' so uid stays as getuid(), pw stays NULL
# group is set but group[0]=='\0', so group block is skipped
# pw==NULL => setgroups(1, &gid) which fails without root
###############################################################################
echo "Test 19 started"
test19_log=""
test19_pass=true

LD_PRELOAD=libtracer.so TRACE_OUTPUT=$PWD/flow_results/test19_trace.log ./su-exec_t19 : echo "test" > "$ACTUAL_DIR/t19_stdout.txt" 2> "$ACTUAL_DIR/t19_stderr.txt"
test19_exit=$?

if [ "$CURRENT_UID" -eq 0 ]; then
    # As root, empty user means current uid (0), empty group means current gid
    if [ "$test19_exit" -ne 0 ]; then
        test19_pass=false
        test19_log="${test19_log}Exit code: expected=0, actual=$test19_exit\n"
    fi
else
    # Not root: setgroups will fail
    if [ "$test19_exit" -ne 1 ]; then
        test19_pass=false
        test19_log="${test19_log}Exit code: expected=1 (non-root), actual=$test19_exit\n"
    fi
    if [ ! -s "$ACTUAL_DIR/t19_stderr.txt" ]; then
        test19_pass=false
        test19_log="${test19_log}Stderr is empty, expected error message\n"
    fi
fi

if [ "$test19_pass" = true ]; then
    echo "Test 19 passed"
    echo "Test 19 passed" > "$RESULTS_DIR/test19_success.log"
else
    echo "Test 19 failed"
    echo "Test 19 failed" >&2
    echo -e "$test19_log" > "$RESULTS_DIR/test19_fail.log"
    failed=1
fi
echo "Test 19 ended"

###############################################################################
# Test 20: Test with root running a command with multiple arguments
# When not root: setgroups fails
###############################################################################
echo "Test 20 started"
test20_log=""
test20_pass=true

LD_PRELOAD=libtracer.so TRACE_OUTPUT=$PWD/flow_results/test20_trace.log ./su-exec_t20 root echo "arg1" "arg2" "arg3" > "$ACTUAL_DIR/t20_stdout.txt" 2> "$ACTUAL_DIR/t20_stderr.txt"
test20_exit=$?

if [ "$CURRENT_UID" -eq 0 ]; then
    if [ "$test20_exit" -ne 0 ]; then
        test20_pass=false
        test20_log="${test20_log}Exit code: expected=0, actual=$test20_exit\n"
    fi
    actual_stdout=$(cat "$ACTUAL_DIR/t20_stdout.txt")
    if [ "$actual_stdout" != "arg1 arg2 arg3" ]; then
        test20_pass=false
        test20_log="${test20_log}Stdout: expected='arg1 arg2 arg3', actual='$actual_stdout'\n"
    fi
else
    # Not root: setgroups will fail
    if [ "$test20_exit" -ne 1 ]; then
        test20_pass=false
        test20_log="${test20_log}Exit code: expected=1 (non-root), actual=$test20_exit\n"
    fi
    if ! grep -q "setgroups" "$ACTUAL_DIR/t20_stderr.txt"; then
        test20_pass=false
        test20_log="${test20_log}Stderr does not contain 'setgroups' error\n"
    fi
fi

if [ "$test20_pass" = true ]; then
    echo "Test 20 passed"
    echo "Test 20 passed" > "$RESULTS_DIR/test20_success.log"
else
    echo "Test 20 failed"
    echo "Test 20 failed" >&2
    echo -e "$test20_log" > "$RESULTS_DIR/test20_fail.log"
    failed=1
fi
echo "Test 20 ended"

###############################################################################
# Summary
###############################################################################
exit $failed

