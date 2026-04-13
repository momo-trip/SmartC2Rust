
#!/bin/bash

# Reformed test cases
failed=0

# Setup
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
mkdir -p flow_results

# Use the local libtracer.so built by c_build.sh
LIBTRACER_PATH="$PWD/libtracer.so"

###############################################################################
# Test 1: help-version (--help and --version)
###############################################################################
echo "Test 1 started"
test1_pass=1
test1_log_file=$(mktemp)

# Test --help: should exit 0 and produce output
LD_PRELOAD=$LIBTRACER_PATH TRACE_OUTPUT=$PWD/flow_results/test1_trace.log ./time_t1 --help > test1_help_stdout.tmp 2> test1_help_stderr.tmp
help_rc=$?
echo "--help exit code: $help_rc" >> "$test1_log_file"
echo "--help stdout:" >> "$test1_log_file"
cat test1_help_stdout.tmp >> "$test1_log_file" 2>/dev/null
echo "" >> "$test1_log_file"
echo "--help stderr:" >> "$test1_log_file"
cat test1_help_stderr.tmp >> "$test1_log_file" 2>/dev/null
echo "" >> "$test1_log_file"
if [ $help_rc -ne 0 ]; then
    echo "FAIL: --help returned non-zero exit code $help_rc" >> "$test1_log_file"
    test1_pass=0
fi
# Check that help output contains 'Usage'
if ! cat test1_help_stdout.tmp test1_help_stderr.tmp 2>/dev/null | grep -qi 'usage'; then
    echo "FAIL: --help output does not contain 'Usage'" >> "$test1_log_file"
    test1_pass=0
fi

# Test --version: should exit 0 and produce output
LD_PRELOAD=$LIBTRACER_PATH TRACE_OUTPUT=$PWD/flow_results/test1_trace.log ./time_t1 --version > test1_ver_stdout.tmp 2> test1_ver_stderr.tmp
version_rc=$?
echo "--version exit code: $version_rc" >> "$test1_log_file"
echo "--version stdout:" >> "$test1_log_file"
cat test1_ver_stdout.tmp >> "$test1_log_file" 2>/dev/null
echo "" >> "$test1_log_file"
echo "--version stderr:" >> "$test1_log_file"
cat test1_ver_stderr.tmp >> "$test1_log_file" 2>/dev/null
echo "" >> "$test1_log_file"
if [ $version_rc -ne 0 ]; then
    echo "FAIL: --version returned non-zero exit code $version_rc" >> "$test1_log_file"
    test1_pass=0
fi

rm -f test1_help_stdout.tmp test1_help_stderr.tmp test1_ver_stdout.tmp test1_ver_stderr.tmp

if [ $test1_pass -eq 1 ]; then
    cp "$test1_log_file" flow_results/test1_success.log
    echo "Test 1 passed"
else
    cp "$test1_log_file" flow_results/test1_fail.log
    echo "Test 1 failed"
    echo "Test 1 failed" >&2
    failed=1
fi
rm -f "$test1_log_file"
echo "Test 1 ended"

###############################################################################
# Test 2: time-max-rss (max RSS reporting with memory allocation)
###############################################################################
echo "Test 2 started"
test2_pass=1
test2_log_file=$(mktemp)

# Get baseline MAX-RSS
LD_PRELOAD=$LIBTRACER_PATH TRACE_OUTPUT=$PWD/flow_results/test2_trace.log ./time_t2 -o mem-baseline -f "%M" ./tests/time-aux_t2 > /dev/null 2>&1
baseline_rc=$?
echo "baseline run exit code: $baseline_rc" >> "$test2_log_file"

# Allocate 50MB of RAM
LD_PRELOAD=$LIBTRACER_PATH TRACE_OUTPUT=$PWD/flow_results/test2_trace.log ./time_t2 -o mem-50MB -f "%M" ./tests/time-aux_t2 -m 50M > /dev/null 2>&1
alloc_rc=$?
echo "50MB alloc run exit code: $alloc_rc" >> "$test2_log_file"

b=""
c=""
if [ -f mem-baseline ]; then
    b=$(cat mem-baseline 2>/dev/null | tr -d '[:space:]')
fi
if [ -f mem-50MB ]; then
    c=$(cat mem-50MB 2>/dev/null | tr -d '[:space:]')
fi
echo "mem-baseline(kb): '$b'" >> "$test2_log_file"
echo "mem-50MB(kb): '$c'" >> "$test2_log_file"

if [ -z "$b" ] || [ -z "$c" ]; then
    echo "FAIL: could not read mem-baseline or mem-50MB" >> "$test2_log_file"
    test2_pass=0
else
    if echo "$b" | grep -qE '^[0-9]+$' && echo "$c" | grep -qE '^[0-9]+$'; then
        d=$(( c - b ))
        echo "delta(kb): $d" >> "$test2_log_file"
        if [ "$b" -eq 0 ] && [ "$c" -eq 0 ]; then
            echo "SKIP: getrusage(2) returns zero in ru_maxrss" >> "$test2_log_file"
        elif [ "$d" -ge 40000 ] && [ "$d" -le 70000 ]; then
            echo "RSS delta is in acceptable range" >> "$test2_log_file"
        else
            echo "FAIL: RSS delta $d not in acceptable range" >> "$test2_log_file"
            test2_pass=0
        fi
    else
        echo "FAIL: non-numeric values: baseline='$b' 50MB='$c'" >> "$test2_log_file"
        test2_pass=0
    fi
fi

rm -f mem-baseline mem-50MB

if [ $test2_pass -eq 1 ]; then
    cp "$test2_log_file" flow_results/test2_success.log
    echo "Test 2 passed"
else
    cp "$test2_log_file" flow_results/test2_fail.log
    echo "Test 2 failed"
    echo "Test 2 failed" >&2
    failed=1
fi
rm -f "$test2_log_file"
echo "Test 2 ended"

###############################################################################
# Test 3: time-exit-codes (exit code propagation)
###############################################################################
echo "Test 3 started"
test3_pass=1
test3_log_file=$(mktemp)

for i in 0 1 3 5 42 100 125; do
    LD_PRELOAD=$LIBTRACER_PATH TRACE_OUTPUT=$PWD/flow_results/test3_trace.log ./time_t3 -q -o out_exit_${i} -f "%x" ./tests/time-aux_t3 -e "$i" > /dev/null 2>&1
    actual_rc=$?
    echo "exit code test $i: expected=$i actual=$actual_rc" >> "$test3_log_file"
    if [ $actual_rc -ne $i ]; then
        echo "FAIL: expected exit code $i, got $actual_rc" >> "$test3_log_file"
        test3_pass=0
    fi
    if [ -f "out_exit_${i}" ]; then
        out_content=$(cat out_exit_${i} | tr -d '[:space:]')
        if [ "$out_content" != "$i" ]; then
            echo "FAIL: output file mismatch for exit $i: expected='$i' got='$out_content'" >> "$test3_log_file"
            test3_pass=0
        fi
    else
        echo "FAIL: output file out_exit_${i} not created" >> "$test3_log_file"
        test3_pass=0
    fi
    rm -f out_exit_${i}
done

if [ $test3_pass -eq 1 ]; then
    cp "$test3_log_file" flow_results/test3_success.log
    echo "Test 3 passed"
else
    cp "$test3_log_file" flow_results/test3_fail.log
    echo "Test 3 failed"
    echo "Test 3 failed" >&2
    failed=1
fi
rm -f "$test3_log_file"
echo "Test 3 ended"

###############################################################################
# Test 4: time-posix-quiet (POSIX output format and quiet mode)
###############################################################################
echo "Test 4 started"
test4_pass=1
test4_log_file=$(mktemp)

# Test POSIX output format (-p flag)
LD_PRELOAD=$LIBTRACER_PATH TRACE_OUTPUT=$PWD/flow_results/test4_trace.log ./time_t4 -p -o out-posix1 true > /dev/null 2>&1
p_rc=$?
echo "-p format run exit code: $p_rc" >> "$test4_log_file"

if [ -f out-posix1 ]; then
    echo "-p output:" >> "$test4_log_file"
    cat out-posix1 >> "$test4_log_file"
    echo "" >> "$test4_log_file"
    if ! grep -q 'real' out-posix1; then
        echo "FAIL: POSIX output does not contain 'real'" >> "$test4_log_file"
        test4_pass=0
    fi
    if ! grep -q 'user' out-posix1; then
        echo "FAIL: POSIX output does not contain 'user'" >> "$test4_log_file"
        test4_pass=0
    fi
    if ! grep -q 'sys' out-posix1; then
        echo "FAIL: POSIX output does not contain 'sys'" >> "$test4_log_file"
        test4_pass=0
    fi
else
    echo "FAIL: out-posix1 not created" >> "$test4_log_file"
    test4_pass=0
fi

# Test quiet mode (-q flag)
LD_PRELOAD=$LIBTRACER_PATH TRACE_OUTPUT=$PWD/flow_results/test4_trace.log ./time_t4 -q -o out-quiet1 false > /dev/null 2>&1
LD_PRELOAD=$LIBTRACER_PATH TRACE_OUTPUT=$PWD/flow_results/test4_trace.log ./time_t4 -o out-noquiet1 false > /dev/null 2>&1

if [ -f out-quiet1 ] && [ -f out-noquiet1 ]; then
    echo "-q output:" >> "$test4_log_file"
    cat out-quiet1 >> "$test4_log_file"
    echo "" >> "$test4_log_file"
    echo "no-q output:" >> "$test4_log_file"
    cat out-noquiet1 >> "$test4_log_file"
    echo "" >> "$test4_log_file"
    if grep -q 'Command exited' out-quiet1; then
        echo "FAIL: -q mode still shows 'Command exited' message" >> "$test4_log_file"
        test4_pass=0
    fi
else
    echo "FAIL: out-quiet1 or out-noquiet1 not created" >> "$test4_log_file"
    test4_pass=0
fi

rm -f out-posix1 out-quiet1 out-noquiet1

if [ $test4_pass -eq 1 ]; then
    cp "$test4_log_file" flow_results/test4_success.log
    echo "Test 4 passed"
else
    cp "$test4_log_file" flow_results/test4_fail.log
    echo "Test 4 failed"
    echo "Test 4 failed" >&2
    failed=1
fi
rm -f "$test4_log_file"
echo "Test 4 ended"

exit $failed

