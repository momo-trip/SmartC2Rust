
#!/bin/bash

# Reformed test cases

# Resolve script directory so tests run relative to this file's location.
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$SCRIPT_DIR" || exit 1

# Sanity check: ensure we are in the time-1.9 directory.
if [ ! -f "./time" ] || [ ! -d "./tests" ]; then
    echo "Error: Please run this script from the time_1_9 directory" >&2
    exit 1
fi

mkdir -p flow_results

failed=0

export VERSION="1.9"
export srcdir="tests"
export builddir="."

#############################################
# Test 1: help-version.sh
#############################################
echo "Test 1 started"
rm -rf flow_results/t1_work
mkdir -p flow_results/t1_work
cp tests/init.sh flow_results/t1_work/init.sh
cp tests/help-version.sh flow_results/t1_work/help-version.sh
# Provide the traced 'time' binary under the name expected by PATH lookup.
cp time_t1 flow_results/t1_work/time
cp tests/time-aux_t1 flow_results/t1_work/time-aux
chmod +x flow_results/t1_work/time flow_results/t1_work/time-aux
T1_DIR="$SCRIPT_DIR/flow_results/t1_work"
LD_PRELOAD=libtracer.so TRACE_OUTPUT="$SCRIPT_DIR/flow_results/test1_trace.log" \
    env PATH="$T1_DIR:$PATH" VERSION="1.9" test_dir="$T1_DIR" \
    sh "$T1_DIR/help-version.sh" > flow_results/test1_output.log 2>&1
t1_rc=$?
echo "Test 1 ended"
if [ "$t1_rc" -eq 0 ]; then
    echo "Test 1 passed"
    cp flow_results/test1_output.log flow_results/test1_success.log
    rm -f flow_results/test1_fail.log
else
    echo "Test 1 failed"
    echo "Test 1 failed" >&2
    cp flow_results/test1_output.log flow_results/test1_fail.log
    rm -f flow_results/test1_success.log
    failed=1
fi

#############################################
# Test 2: time-max-rss.sh
#############################################
echo "Test 2 started"
rm -rf flow_results/t2_work
mkdir -p flow_results/t2_work
cp tests/init.sh flow_results/t2_work/init.sh
cp tests/time-max-rss.sh flow_results/t2_work/time-max-rss.sh
cp time_t2 flow_results/t2_work/time
cp tests/time-aux_t2 flow_results/t2_work/time-aux
chmod +x flow_results/t2_work/time flow_results/t2_work/time-aux
T2_DIR="$SCRIPT_DIR/flow_results/t2_work"
LD_PRELOAD=libtracer.so TRACE_OUTPUT="$SCRIPT_DIR/flow_results/test2_trace.log" \
    env PATH="$T2_DIR:$PATH" VERSION="1.9" test_dir="$T2_DIR" \
    sh "$T2_DIR/time-max-rss.sh" > flow_results/test2_output.log 2>&1
t2_rc=$?
echo "Test 2 ended"
if [ "$t2_rc" -eq 0 ]; then
    echo "Test 2 passed"
    cp flow_results/test2_output.log flow_results/test2_success.log
    rm -f flow_results/test2_fail.log
else
    echo "Test 2 failed"
    echo "Test 2 failed" >&2
    cp flow_results/test2_output.log flow_results/test2_fail.log
    rm -f flow_results/test2_success.log
    failed=1
fi

#############################################
# Test 3: time-exit-codes.sh
#############################################
echo "Test 3 started"
rm -rf flow_results/t3_work
mkdir -p flow_results/t3_work
cp tests/init.sh flow_results/t3_work/init.sh
cp tests/time-exit-codes.sh flow_results/t3_work/time-exit-codes.sh
cp time_t3 flow_results/t3_work/time
cp tests/time-aux_t3 flow_results/t3_work/time-aux
chmod +x flow_results/t3_work/time flow_results/t3_work/time-aux
T3_DIR="$SCRIPT_DIR/flow_results/t3_work"
LD_PRELOAD=libtracer.so TRACE_OUTPUT="$SCRIPT_DIR/flow_results/test3_trace.log" \
    env PATH="$T3_DIR:$PATH" VERSION="1.9" test_dir="$T3_DIR" \
    sh "$T3_DIR/time-exit-codes.sh" > flow_results/test3_output.log 2>&1
t3_rc=$?
echo "Test 3 ended"
if [ "$t3_rc" -eq 0 ]; then
    echo "Test 3 passed"
    cp flow_results/test3_output.log flow_results/test3_success.log
    rm -f flow_results/test3_fail.log
else
    echo "Test 3 failed"
    echo "Test 3 failed" >&2
    cp flow_results/test3_output.log flow_results/test3_fail.log
    rm -f flow_results/test3_success.log
    failed=1
fi

#############################################
# Test 4: time-posix-quiet.sh
#############################################
echo "Test 4 started"
rm -rf flow_results/t4_work
mkdir -p flow_results/t4_work
cp tests/init.sh flow_results/t4_work/init.sh
cp tests/time-posix-quiet.sh flow_results/t4_work/time-posix-quiet.sh
cp time_t4 flow_results/t4_work/time
cp tests/time-aux_t4 flow_results/t4_work/time-aux
chmod +x flow_results/t4_work/time flow_results/t4_work/time-aux
T4_DIR="$SCRIPT_DIR/flow_results/t4_work"
LD_PRELOAD=libtracer.so TRACE_OUTPUT="$SCRIPT_DIR/flow_results/test4_trace.log" \
    env PATH="$T4_DIR:$PATH" VERSION="1.9" test_dir="$T4_DIR" \
    sh "$T4_DIR/time-posix-quiet.sh" > flow_results/test4_output.log 2>&1
t4_rc=$?
echo "Test 4 ended"
if [ "$t4_rc" -eq 0 ]; then
    echo "Test 4 passed"
    cp flow_results/test4_output.log flow_results/test4_success.log
    rm -f flow_results/test4_fail.log
else
    echo "Test 4 failed"
    echo "Test 4 failed" >&2
    cp flow_results/test4_output.log flow_results/test4_fail.log
    rm -f flow_results/test4_success.log
    failed=1
fi

exit $failed

