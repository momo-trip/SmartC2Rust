
#!/bin/bash

# Reformed test cases

failed=0
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$SCRIPT_DIR"

mkdir -p flow_results

export VERSION="1.9"
export PATH="$SCRIPT_DIR:$SCRIPT_DIR/tests:$PATH"
export LD_LIBRARY_PATH="$SCRIPT_DIR:${LD_LIBRARY_PATH:-}"

# Make sure the 'time' binary exists
if [ ! -x "$SCRIPT_DIR/time" ]; then
    echo "Error: time binary not found at $SCRIPT_DIR/time" >&2
    exit 1
fi

# Make sure tests/time-aux exists
if [ ! -x "$SCRIPT_DIR/tests/time-aux" ]; then
    echo "Error: tests/time-aux binary not found" >&2
    exit 1
fi

#############################################
# Test 1: help-version test
#############################################
test_num=1
echo "Test ${test_num} started"
log_file="flow_results/test${test_num}.log"
: > "$log_file"

t1_ok=1

# Run --version and check it matches $VERSION
LD_PRELOAD=libtracer.so TRACE_OUTPUT=$PWD/flow_results/test${test_num}_trace.log "$SCRIPT_DIR/time" --version >"flow_results/test${test_num}_version.out" 2>>"$log_file"
rc=$?
echo "--version exit: $rc" >> "$log_file"
if [ $rc -ne 0 ]; then
    t1_ok=0
fi

v=$(sed -n -e '1s/.* //p' -e 'q' "flow_results/test${test_num}_version.out")
echo "version detected: '$v' expected: '$VERSION'" >> "$log_file"
if [ "x$v" != "x$VERSION" ]; then
    t1_ok=0
fi

# --help should succeed
LD_PRELOAD=libtracer.so TRACE_OUTPUT=$PWD/flow_results/test${test_num}_trace_help.log "$SCRIPT_DIR/time" --help >/dev/null 2>>"$log_file"
rc=$?
echo "--help exit: $rc" >> "$log_file"
if [ $rc -ne 0 ]; then
    t1_ok=0
fi

if [ $t1_ok -eq 1 ]; then
    echo "Test ${test_num} passed"
    mv "$log_file" "flow_results/test${test_num}_success.log"
else
    echo "Test ${test_num} failed" >&2
    mv "$log_file" "flow_results/test${test_num}_fail.log"
    failed=1
fi
echo "Test ${test_num} ended"

#############################################
# Test 2: time-exit-codes test
#############################################
test_num=2
echo "Test ${test_num} started"
log_file="flow_results/test${test_num}.log"
: > "$log_file"

t2_ok=1
workdir="flow_results/t2_work"
rm -rf "$workdir"
mkdir -p "$workdir"

for i in 0 1 3 5 100 123 125 126 128 ; do
    echo "--- testing exit code $i ---" >> "$log_file"
    printf "%d\n" "$i" > "$workdir/exp$i"
    LD_PRELOAD=libtracer.so TRACE_OUTPUT=$PWD/flow_results/test${test_num}_trace_${i}.log "$SCRIPT_DIR/time" -q -o "$workdir/out$i" -f "%x" "$SCRIPT_DIR/tests/time-aux" -e "$i" >>"$log_file" 2>&1
    rc=$?
    echo "time exit rc=$rc (expected $i)" >> "$log_file"
    if [ "$rc" -ne "$i" ]; then
        t2_ok=0
    fi
    if ! diff "$workdir/exp$i" "$workdir/out$i" >>"$log_file" 2>&1; then
        t2_ok=0
    fi
done

if [ $t2_ok -eq 1 ]; then
    echo "Test ${test_num} passed"
    mv "$log_file" "flow_results/test${test_num}_success.log"
else
    echo "Test ${test_num} failed" >&2
    mv "$log_file" "flow_results/test${test_num}_fail.log"
    failed=1
fi
echo "Test ${test_num} ended"

#############################################
# Test 3: time-max-rss test
#############################################
test_num=3
echo "Test ${test_num} started"
log_file="flow_results/test${test_num}.log"
: > "$log_file"

t3_ok=1
workdir="flow_results/t3_work"
rm -rf "$workdir"
mkdir -p "$workdir"

LD_PRELOAD=libtracer.so TRACE_OUTPUT=$PWD/flow_results/test${test_num}_trace_baseline.log "$SCRIPT_DIR/time" -o "$workdir/mem-baseline" -f "%M" "$SCRIPT_DIR/tests/time-aux" >>"$log_file" 2>&1
rc=$?
echo "baseline exit rc=$rc" >> "$log_file"
if [ $rc -ne 0 ]; then
    t3_ok=0
fi

LD_PRELOAD=libtracer.so TRACE_OUTPUT=$PWD/flow_results/test${test_num}_trace_50MB.log "$SCRIPT_DIR/time" -o "$workdir/mem-50MB" -f "%M" "$SCRIPT_DIR/tests/time-aux" -m 50M >>"$log_file" 2>&1
rc=$?
echo "50MB exit rc=$rc" >> "$log_file"
if [ $rc -ne 0 ]; then
    t3_ok=0
fi

if [ $t3_ok -eq 1 ]; then
    b=$(cat "$workdir/mem-baseline")
    c=$(cat "$workdir/mem-50MB")
    d=$(( c - b ))
    echo "baseline=$b mem50=$c delta=$d" >> "$log_file"
    if [ "$b" -eq 0 ] && [ "$c" -eq 0 ]; then
        echo "getrusage(2) returns zero in ru_maxrss - skipping accuracy check" >> "$log_file"
    else
        if [ "$d" -ge 40000 ] && [ "$d" -le 60000 ]; then
            echo "delta within expected range" >> "$log_file"
        else
            echo "delta out of range (expected 40000-60000, got $d)" >> "$log_file"
            t3_ok=0
        fi
    fi
fi

if [ $t3_ok -eq 1 ]; then
    echo "Test ${test_num} passed"
    mv "$log_file" "flow_results/test${test_num}_success.log"
else
    echo "Test ${test_num} failed" >&2
    mv "$log_file" "flow_results/test${test_num}_fail.log"
    failed=1
fi
echo "Test ${test_num} ended"

#############################################
# Test 4: time-posix-quiet test
#############################################
test_num=4
echo "Test ${test_num} started"
log_file="flow_results/test${test_num}.log"
: > "$log_file"

t4_ok=1
workdir="flow_results/t4_work"
rm -rf "$workdir"
mkdir -p "$workdir"

remove_numeric_values() {
    sed -e 's/[?0-9.]*//g' -e 's/ *$//' "$@"
}

if ! command -v false >/dev/null; then
    echo "'false' not available" >> "$log_file"
    t4_ok=0
fi
if ! command -v sed >/dev/null; then
    echo "'sed' not available" >> "$log_file"
    t4_ok=0
fi

# Default output
cat <<EOF > "$workdir/exp-default"
Command exited with non-zero status
user system :elapsed %CPU (avgtext+avgdata maxresident)k
inputs+outputs (major+minor)pagefaults swaps
EOF

LD_PRELOAD=libtracer.so TRACE_OUTPUT=$PWD/flow_results/test${test_num}_trace_def.log "$SCRIPT_DIR/time" -o "$workdir/out-def1" false >>"$log_file" 2>&1
rc=$?
echo "default time(false) rc=$rc (expected 1)" >> "$log_file"
if [ $rc -ne 1 ]; then
    t4_ok=0
fi

remove_numeric_values "$workdir/out-def1" > "$workdir/out-default"
if ! diff "$workdir/out-default" "$workdir/exp-default" >>"$log_file" 2>&1; then
    t4_ok=0
fi

# -q output
cat <<EOF > "$workdir/exp-q"
user system :elapsed %CPU (avgtext+avgdata maxresident)k
inputs+outputs (major+minor)pagefaults swaps
EOF

LD_PRELOAD=libtracer.so TRACE_OUTPUT=$PWD/flow_results/test${test_num}_trace_q.log "$SCRIPT_DIR/time" -q -o "$workdir/out-q1" false >>"$log_file" 2>&1
rc=$?
echo "-q time(false) rc=$rc (expected 1)" >> "$log_file"
if [ $rc -ne 1 ]; then
    t4_ok=0
fi

remove_numeric_values "$workdir/out-q1" > "$workdir/out-q"
if ! diff "$workdir/out-q" "$workdir/exp-q" >>"$log_file" 2>&1; then
    t4_ok=0
fi

# -p (POSIX) output
cat <<EOF > "$workdir/exp-posix"
real
user
sys
EOF

LD_PRELOAD=libtracer.so TRACE_OUTPUT=$PWD/flow_results/test${test_num}_trace_posix.log "$SCRIPT_DIR/time" -p -o "$workdir/out-posix1" false >>"$log_file" 2>&1
rc=$?
echo "-p time(false) rc=$rc (expected 1)" >> "$log_file"
if [ $rc -ne 1 ]; then
    t4_ok=0
fi

remove_numeric_values "$workdir/out-posix1" > "$workdir/out-posix"
if ! diff "$workdir/out-posix" "$workdir/exp-posix" >>"$log_file" 2>&1; then
    t4_ok=0
fi

if [ $t4_ok -eq 1 ]; then
    echo "Test ${test_num} passed"
    mv "$log_file" "flow_results/test${test_num}_success.log"
else
    echo "Test ${test_num} failed" >&2
    mv "$log_file" "flow_results/test${test_num}_fail.log"
    failed=1
fi
echo "Test ${test_num} ended"

exit $failed

