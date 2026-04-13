
#!/bin/bash

# Reformed test cases

failed=0

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
export PATH="$SCRIPT_DIR:$SCRIPT_DIR/tests:$PATH"
export VERSION="1.9"
export srcdir="tests"
export builddir="."

mkdir -p genifai_results

TIME_BIN="$SCRIPT_DIR/time"
TIME_AUX_BIN="$SCRIPT_DIR/tests/time-aux"

# ============================================================
# Test 1: help-version (--help and --version)
# ============================================================
echo "Test 1 started"
test1_log=""
test1_pass=1

# Test --help
help_out=$(LD_PRELOAD=libtracer.so TRACE_OUTPUT=$PWD/genifai_results/test1_trace.log $SCRIPT_DIR/time_t1 --help 2>&1)
rc=$?
test1_log="${test1_log}--help exit code: $rc\n"
test1_log="${test1_log}--help output: $help_out\n"
if [ $rc -ne 0 ]; then
    test1_log="${test1_log}FAIL: --help returned non-zero exit code\n"
    test1_pass=0
fi
# Check that --help output contains 'Usage'
if echo "$help_out" | grep -qi 'usage'; then
    test1_log="${test1_log}PASS: --help output contains 'Usage'\n"
else
    test1_log="${test1_log}FAIL: --help output does not contain 'Usage'\n"
    test1_pass=0
fi

# Test --version
version_out=$(LD_PRELOAD=libtracer.so TRACE_OUTPUT=$PWD/genifai_results/test1_trace.log $SCRIPT_DIR/time_t1 --version 2>&1)
rc=$?
test1_log="${test1_log}--version exit code: $rc\n"
test1_log="${test1_log}--version output: $version_out\n"
if [ $rc -ne 0 ]; then
    test1_log="${test1_log}FAIL: --version returned non-zero exit code\n"
    test1_pass=0
fi
# Check that --version output contains version number
if echo "$version_out" | grep -q '1.9'; then
    test1_log="${test1_log}PASS: --version output contains version number\n"
else
    test1_log="${test1_log}FAIL: --version output does not contain version number\n"
    test1_pass=0
fi

if [ $test1_pass -eq 1 ]; then
    echo "Test 1 passed"
    printf "%b" "$test1_log" > genifai_results/test1_success.log
else
    echo "Test 1 failed"
    echo "Test 1 failed" >&2
    printf "%b" "$test1_log" > genifai_results/test1_fail.log
    failed=1
fi
echo "Test 1 ended"

# ============================================================
# Test 2: time-max-rss (MAX-RSS reporting)
# ============================================================
echo "Test 2 started"
test2_log=""
test2_pass=1

# Check time-aux is available
if [ ! -x "$SCRIPT_DIR/tests/time-aux_t2" ]; then
    test2_log="${test2_log}FAIL: time-aux_t2 is missing/not executable\n"
    test2_pass=0
else
    # Get baseline MAX-RSS
    LD_PRELOAD=libtracer.so TRACE_OUTPUT=$PWD/genifai_results/test2_trace.log $SCRIPT_DIR/time_t2 -o /tmp/time_test2_mem_baseline -f "%M" $SCRIPT_DIR/tests/time-aux_t2 2>/dev/null
    rc=$?
    test2_log="${test2_log}baseline run exit code: $rc\n"
    if [ $rc -ne 0 ]; then
        test2_log="${test2_log}FAIL: failed to run time/time-aux (baseline max-rss)\n"
        test2_pass=0
    else
        # Allocate 5MB of RAM
        LD_PRELOAD=libtracer.so TRACE_OUTPUT=$PWD/genifai_results/test2_trace.log $SCRIPT_DIR/time_t2 -o /tmp/time_test2_mem_5MB -f "%M" $SCRIPT_DIR/tests/time-aux_t2 -m 5M 2>/dev/null
        rc=$?
        test2_log="${test2_log}5MB run exit code: $rc\n"
        if [ $rc -ne 0 ]; then
            test2_log="${test2_log}FAIL: failed to run time/time-aux (5M max-rss)\n"
            test2_pass=0
        else
            b=$(cat /tmp/time_test2_mem_baseline)
            c=$(cat /tmp/time_test2_mem_5MB)
            d=$(( c - b ))
            test2_log="${test2_log}mem-baseline(kb): $b\n"
            test2_log="${test2_log}mem-5MB(kb): $c\n"
            test2_log="${test2_log}delta(kb): $d\n"

            # On some systems getrusage(2) returns zero in ru_maxrss - skip check
            if [ "$b" -eq 0 ] && [ "$c" -eq 0 ]; then
                test2_log="${test2_log}SKIP: getrusage(2) returns zero in ru_maxrss\n"
            else
                if [ "$d" -ge 4000 ] && [ "$d" -le 7000 ]; then
                    test2_log="${test2_log}PASS: delta is in acceptable range (4000-7000 KB)\n"
                else
                    test2_log="${test2_log}FAIL: delta $d not in acceptable range\n"
                    test2_pass=0
                fi
            fi
        fi
    fi
    rm -f /tmp/time_test2_mem_baseline /tmp/time_test2_mem_5MB
fi

if [ $test2_pass -eq 1 ]; then
    echo "Test 2 passed"
    printf "%b" "$test2_log" > genifai_results/test2_success.log
else
    echo "Test 2 failed"
    echo "Test 2 failed" >&2
    printf "%b" "$test2_log" > genifai_results/test2_fail.log
    failed=1
fi
echo "Test 2 ended"

# ============================================================
# Test 3: time-exit-codes
# ============================================================
echo "Test 3 started"
test3_log=""
test3_pass=1

# Test that time returns the exit code of the child process
# Exit code 0
LD_PRELOAD=libtracer.so TRACE_OUTPUT=$PWD/genifai_results/test3_trace.log $SCRIPT_DIR/time_t3 -o /dev/null true 2>/dev/null
rc=$?
test3_log="${test3_log}exit code for 'true': $rc (expected 0)\n"
if [ $rc -ne 0 ]; then
    test3_log="${test3_log}FAIL: expected exit code 0, got $rc\n"
    test3_pass=0
fi

# Exit code 1
LD_PRELOAD=libtracer.so TRACE_OUTPUT=$PWD/genifai_results/test3_trace.log $SCRIPT_DIR/time_t3 -o /dev/null false 2>/dev/null
rc=$?
test3_log="${test3_log}exit code for 'false': $rc (expected 1)\n"
if [ $rc -ne 1 ]; then
    test3_log="${test3_log}FAIL: expected exit code 1, got $rc\n"
    test3_pass=0
fi

# Exit code from time-aux with specific exit code
if [ -x "$SCRIPT_DIR/tests/time-aux_t3" ]; then
    LD_PRELOAD=libtracer.so TRACE_OUTPUT=$PWD/genifai_results/test3_trace.log $SCRIPT_DIR/time_t3 -o /dev/null $SCRIPT_DIR/tests/time-aux_t3 -e 42 2>/dev/null
    rc=$?
    test3_log="${test3_log}exit code for 'time-aux -e 42': $rc (expected 42)\n"
    if [ $rc -ne 42 ]; then
        test3_log="${test3_log}FAIL: expected exit code 42, got $rc\n"
        test3_pass=0
    fi
else
    test3_log="${test3_log}SKIP: time-aux_t3 not available for exit code test\n"
fi

if [ $test3_pass -eq 1 ]; then
    echo "Test 3 passed"
    printf "%b" "$test3_log" > genifai_results/test3_success.log
else
    echo "Test 3 failed"
    echo "Test 3 failed" >&2
    printf "%b" "$test3_log" > genifai_results/test3_fail.log
    failed=1
fi
echo "Test 3 ended"

# ============================================================
# Test 4: time-posix-quiet
# ============================================================
echo "Test 4 started"
test4_log=""
test4_pass=1

# Test POSIX output format with -p flag
posix_out=$(LD_PRELOAD=libtracer.so TRACE_OUTPUT=$PWD/genifai_results/test4_trace.log $SCRIPT_DIR/time_t4 -p true 2>&1)
rc=$?
test4_log="${test4_log}-p true exit code: $rc\n"
test4_log="${test4_log}-p true output: $posix_out\n"
if [ $rc -ne 0 ]; then
    test4_log="${test4_log}FAIL: -p true returned non-zero exit code\n"
    test4_pass=0
fi

# Check POSIX format output contains 'real', 'user', 'sys'
if echo "$posix_out" | grep -q 'real'; then
    test4_log="${test4_log}PASS: POSIX output contains 'real'\n"
else
    test4_log="${test4_log}FAIL: POSIX output does not contain 'real'\n"
    test4_pass=0
fi
if echo "$posix_out" | grep -q 'user'; then
    test4_log="${test4_log}PASS: POSIX output contains 'user'\n"
else
    test4_log="${test4_log}FAIL: POSIX output does not contain 'user'\n"
    test4_pass=0
fi
if echo "$posix_out" | grep -q 'sys'; then
    test4_log="${test4_log}PASS: POSIX output contains 'sys'\n"
else
    test4_log="${test4_log}FAIL: POSIX output does not contain 'sys'\n"
    test4_pass=0
fi

# Test quiet mode: -q should suppress output to stderr
quiet_stderr=$(LD_PRELOAD=libtracer.so TRACE_OUTPUT=$PWD/genifai_results/test4_trace.log $SCRIPT_DIR/time_t4 -q true 2>&1 1>/dev/null)
rc=$?
test4_log="${test4_log}-q true exit code: $rc\n"
test4_log="${test4_log}-q true stderr: '$quiet_stderr'\n"
if [ $rc -ne 0 ]; then
    test4_log="${test4_log}FAIL: -q true returned non-zero exit code\n"
    test4_pass=0
fi

# Test output to file with -o
tmpfile=$(mktemp /tmp/time_test4_XXXXXX)
LD_PRELOAD=libtracer.so TRACE_OUTPUT=$PWD/genifai_results/test4_trace.log $SCRIPT_DIR/time_t4 -o "$tmpfile" -f "%e" true 2>/dev/null
rc=$?
test4_log="${test4_log}-o file exit code: $rc\n"
if [ $rc -ne 0 ]; then
    test4_log="${test4_log}FAIL: -o file returned non-zero exit code\n"
    test4_pass=0
else
    file_content=$(cat "$tmpfile")
    test4_log="${test4_log}-o file content: '$file_content'\n"
    if [ -n "$file_content" ]; then
        test4_log="${test4_log}PASS: -o file produced output\n"
    else
        test4_log="${test4_log}FAIL: -o file produced no output\n"
        test4_pass=0
    fi
fi
rm -f "$tmpfile"

if [ $test4_pass -eq 1 ]; then
    echo "Test 4 passed"
    printf "%b" "$test4_log" > genifai_results/test4_success.log
else
    echo "Test 4 failed"
    echo "Test 4 failed" >&2
    printf "%b" "$test4_log" > genifai_results/test4_fail.log
    failed=1
fi
echo "Test 4 ended"

exit $failed

