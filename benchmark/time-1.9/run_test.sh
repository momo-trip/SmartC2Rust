
#!/bin/bash

# Reformed test cases

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$SCRIPT_DIR"

mkdir -p flow_results

export VERSION="1.9"
export LC_ALL=C

failed=0

#########################################
# Test 1: help-version test
# Ensure 'time --help' and 'time --version' run successfully
# and version string matches $VERSION
#########################################
echo "Test 1 started"
TEST_LOG="flow_results/test1.tmp.log"
: > "$TEST_LOG"
test1_ok=1

# --version check
LD_PRELOAD=libtracer.so TRACE_OUTPUT=$PWD/flow_results/test1_trace.log ./test_bins/time_t1 --version >"flow_results/test1_ver.out" 2>>"$TEST_LOG"
rc=$?
if [ $rc -ne 0 ]; then
    echo "time --version failed with rc=$rc" >>"$TEST_LOG"
    test1_ok=0
fi
v=$(sed -n -e '1s/.* //p' -e 'q' "flow_results/test1_ver.out")
if [ "x$v" != "x$VERSION" ]; then
    echo "--version-\$VERSION mismatch: got '$v', expected '$VERSION'" >>"$TEST_LOG"
    test1_ok=0
fi

# --help check
LD_PRELOAD=libtracer.so TRACE_OUTPUT=$PWD/flow_results/test1_trace.log ./test_bins/time_t1 --help >/dev/null 2>>"$TEST_LOG"
rc=$?
if [ $rc -ne 0 ]; then
    echo "time --help failed with rc=$rc" >>"$TEST_LOG"
    test1_ok=0
fi

echo "Test 1 ended"
if [ $test1_ok -eq 1 ]; then
    mv "$TEST_LOG" flow_results/test1_success.log
    echo "Test 1 passed"
else
    mv "$TEST_LOG" flow_results/test1_fail.log
    echo "Test 1 failed" >&2
    failed=1
fi
rm -f flow_results/test1_ver.out

#########################################
# Test 2: time-exit-codes test
# Ensure time propagates the correct exit codes from child process
#########################################
echo "Test 2 started"
TEST_LOG="flow_results/test2.tmp.log"
: > "$TEST_LOG"
test2_ok=1

TIME_BIN="./test_bins/time_t2"
AUX_BIN="$PWD/test_bins/time-aux_t2"

# sanity check
LD_PRELOAD=libtracer.so TRACE_OUTPUT=$PWD/flow_results/test2_trace.log "$AUX_BIN" >>"$TEST_LOG" 2>&1
rc=$?
if [ $rc -ne 0 ]; then
    echo "time-aux sanity check failed rc=$rc" >>"$TEST_LOG"
    test2_ok=0
fi

for i in 0 1 3 5 100 123 125 126 128; do
    LD_PRELOAD=libtracer.so TRACE_OUTPUT=$PWD/flow_results/test2_trace.log \
        "$TIME_BIN" -q -o "flow_results/test2_out$i" -f "%x" "$AUX_BIN" -e "$i" >>"$TEST_LOG" 2>&1
    rc=$?
    if [ "$rc" -ne "$i" ]; then
        echo "exit code mismatch: expected $i, got $rc" >>"$TEST_LOG"
        test2_ok=0
    fi
    out_val=$(cat "flow_results/test2_out$i" 2>/dev/null)
    if [ "$out_val" != "$i" ]; then
        echo "reported exit code mismatch for $i: got '$out_val'" >>"$TEST_LOG"
        test2_ok=0
    fi
    rm -f "flow_results/test2_out$i"
done

echo "Test 2 ended"
if [ $test2_ok -eq 1 ]; then
    mv "$TEST_LOG" flow_results/test2_success.log
    echo "Test 2 passed"
else
    mv "$TEST_LOG" flow_results/test2_fail.log
    echo "Test 2 failed" >&2
    failed=1
fi

#########################################
# Test 3: time-posix-quiet test
# Test output quietness with -q and -p
#########################################
echo "Test 3 started"
TEST_LOG="flow_results/test3.tmp.log"
: > "$TEST_LOG"
test3_ok=1

TIME_BIN="./test_bins/time_t3"
TDIR="flow_results/test3_work"
mkdir -p "$TDIR"

remove_numeric_values() {
    sed -e 's/[?0-9.]*//g' -e 's/ *$//' "$@"
}

# Default output
cat <<EOF > "$TDIR/exp-default"
Command exited with non-zero status
user system :elapsed %CPU (avgtext+avgdata maxresident)k
inputs+outputs (major+minor)pagefaults swaps
EOF

LD_PRELOAD=libtracer.so TRACE_OUTPUT=$PWD/flow_results/test3_trace.log "$TIME_BIN" -o "$TDIR/out-def1" false >>"$TEST_LOG" 2>&1
rc=$?
if [ $rc -ne 1 ]; then
    echo "default: expected rc=1, got rc=$rc" >>"$TEST_LOG"
    test3_ok=0
fi
remove_numeric_values "$TDIR/out-def1" > "$TDIR/out-default"
if ! diff -u "$TDIR/exp-default" "$TDIR/out-default" >>"$TEST_LOG" 2>&1; then
    echo "default output mismatch" >>"$TEST_LOG"
    test3_ok=0
fi

# -q output
cat <<EOF > "$TDIR/exp-q"
user system :elapsed %CPU (avgtext+avgdata maxresident)k
inputs+outputs (major+minor)pagefaults swaps
EOF

LD_PRELOAD=libtracer.so TRACE_OUTPUT=$PWD/flow_results/test3_trace.log "$TIME_BIN" -q -o "$TDIR/out-q1" false >>"$TEST_LOG" 2>&1
rc=$?
if [ $rc -ne 1 ]; then
    echo "-q: expected rc=1, got rc=$rc" >>"$TEST_LOG"
    test3_ok=0
fi
remove_numeric_values "$TDIR/out-q1" > "$TDIR/out-q"
if ! diff -u "$TDIR/exp-q" "$TDIR/out-q" >>"$TEST_LOG" 2>&1; then
    echo "-q output mismatch" >>"$TEST_LOG"
    test3_ok=0
fi

# -p output
cat <<EOF > "$TDIR/exp-posix"
real
user
sys
EOF

LD_PRELOAD=libtracer.so TRACE_OUTPUT=$PWD/flow_results/test3_trace.log "$TIME_BIN" -p -o "$TDIR/out-posix1" false >>"$TEST_LOG" 2>&1
rc=$?
if [ $rc -ne 1 ]; then
    echo "-p: expected rc=1, got rc=$rc" >>"$TEST_LOG"
    test3_ok=0
fi
remove_numeric_values "$TDIR/out-posix1" > "$TDIR/out-posix"
if ! diff -u "$TDIR/exp-posix" "$TDIR/out-posix" >>"$TEST_LOG" 2>&1; then
    echo "-p output mismatch" >>"$TEST_LOG"
    test3_ok=0
fi

echo "Test 3 ended"
if [ $test3_ok -eq 1 ]; then
    mv "$TEST_LOG" flow_results/test3_success.log
    echo "Test 3 passed"
else
    mv "$TEST_LOG" flow_results/test3_fail.log
    echo "Test 3 failed" >&2
    failed=1
fi
rm -rf "$TDIR"

#########################################
# Test 4: time-max-rss test
# Test MAX-RSS (Resident size) reporting for 50MB allocation
#########################################
echo "Test 4 started"
TEST_LOG="flow_results/test4.tmp.log"
: > "$TEST_LOG"
test4_ok=1

TIME_BIN="./test_bins/time_t4"
AUX_BIN="$PWD/test_bins/time-aux_t4"
TDIR="flow_results/test4_work"
mkdir -p "$TDIR"

LD_PRELOAD=libtracer.so TRACE_OUTPUT=$PWD/flow_results/test4_trace.log "$AUX_BIN" >>"$TEST_LOG" 2>&1
if [ $? -ne 0 ]; then
    echo "time-aux not runnable" >>"$TEST_LOG"
    test4_ok=0
fi

LD_PRELOAD=libtracer.so TRACE_OUTPUT=$PWD/flow_results/test4_trace.log "$TIME_BIN" -o "$TDIR/mem-baseline" -f "%M" "$AUX_BIN" >>"$TEST_LOG" 2>&1
if [ $? -ne 0 ]; then
    echo "failed to run time/time-aux (baseline max-rss)" >>"$TEST_LOG"
    test4_ok=0
fi

LD_PRELOAD=libtracer.so TRACE_OUTPUT=$PWD/flow_results/test4_trace.log "$TIME_BIN" -o "$TDIR/mem-50MB" -f "%M" "$AUX_BIN" -m 50M >>"$TEST_LOG" 2>&1
if [ $? -ne 0 ]; then
    echo "failed to run time/time-aux (50M max-rss)" >>"$TEST_LOG"
    test4_ok=0
fi

if [ $test4_ok -eq 1 ]; then
    b=$(cat "$TDIR/mem-baseline")
    c=$(cat "$TDIR/mem-50MB")
    d=$(( c - b ))
    echo "baseline=$b, 50MB=$c, delta=$d" >>"$TEST_LOG"
    if [ "$b" -eq 0 ] && [ "$c" -eq 0 ]; then
        echo "getrusage returns zero in ru_maxrss - skipping comparison" >>"$TEST_LOG"
    elif [ "$d" -ge 40000 ] && [ "$d" -le 60000 ]; then
        echo "delta is within acceptable range" >>"$TEST_LOG"
    else
        echo "time(1) failed to detect 50MB allocation: delta=$d" >>"$TEST_LOG"
        test4_ok=0
    fi
fi

echo "Test 4 ended"
if [ $test4_ok -eq 1 ]; then
    mv "$TEST_LOG" flow_results/test4_success.log
    echo "Test 4 passed"
else
    mv "$TEST_LOG" flow_results/test4_fail.log
    echo "Test 4 failed" >&2
    failed=1
fi
rm -rf "$TDIR"

#########################################
# Test 5: time-max-rss0 test
# Test MAX-RSS reporting for 5MB allocation
#########################################
echo "Test 5 started"
TEST_LOG="flow_results/test5.tmp.log"
: > "$TEST_LOG"
test5_ok=1

TIME_BIN="./test_bins/time_t5"
AUX_BIN="$PWD/test_bins/time-aux_t5"
TDIR="flow_results/test5_work"
mkdir -p "$TDIR"

LD_PRELOAD=libtracer.so TRACE_OUTPUT=$PWD/flow_results/test5_trace.log "$AUX_BIN" >>"$TEST_LOG" 2>&1
if [ $? -ne 0 ]; then
    echo "time-aux not runnable" >>"$TEST_LOG"
    test5_ok=0
fi

LD_PRELOAD=libtracer.so TRACE_OUTPUT=$PWD/flow_results/test5_trace.log "$TIME_BIN" -o "$TDIR/mem-baseline" -f "%M" "$AUX_BIN" >>"$TEST_LOG" 2>&1
if [ $? -ne 0 ]; then
    echo "failed to run time/time-aux (baseline max-rss)" >>"$TEST_LOG"
    test5_ok=0
fi

LD_PRELOAD=libtracer.so TRACE_OUTPUT=$PWD/flow_results/test5_trace.log "$TIME_BIN" -o "$TDIR/mem-5MB" -f "%M" "$AUX_BIN" -m 5M >>"$TEST_LOG" 2>&1
if [ $? -ne 0 ]; then
    echo "failed to run time/time-aux (5M max-rss)" >>"$TEST_LOG"
    test5_ok=0
fi

if [ $test5_ok -eq 1 ]; then
    b=$(cat "$TDIR/mem-baseline")
    c=$(cat "$TDIR/mem-5MB")
    d=$(( c - b ))
    echo "baseline=$b, 5MB=$c, delta=$d" >>"$TEST_LOG"
    if [ "$b" -eq 0 ] && [ "$c" -eq 0 ]; then
        echo "getrusage returns zero in ru_maxrss - skipping comparison" >>"$TEST_LOG"
    elif [ "$d" -ge 5000 ] && [ "$d" -le 6000 ]; then
        echo "delta is within acceptable range" >>"$TEST_LOG"
    else
        echo "time(1) failed to detect 5MB allocation: delta=$d" >>"$TEST_LOG"
        test5_ok=0
    fi
fi

echo "Test 5 ended"
if [ $test5_ok -eq 1 ]; then
    mv "$TEST_LOG" flow_results/test5_success.log
    echo "Test 5 passed"
else
    mv "$TEST_LOG" flow_results/test5_fail.log
    echo "Test 5 failed" >&2
    failed=1
fi
rm -rf "$TDIR"

exit $failed

