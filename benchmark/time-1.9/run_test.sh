
#!/bin/bash

# Reformed test cases from base_test.sh
# Runs original test scripts from tests/ directory

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$SCRIPT_DIR" || exit 1

mkdir -p flow_results

failed=0

# Check that we're in the correct directory (from base_test.sh)
if [ ! -f "./time" ] || [ ! -d "./tests" ]; then
    echo "Error: Please run this script from the time_1_9 directory" >&2
    exit 1
fi

# Setup environment for tests (from base_test.sh)
export PATH="$SCRIPT_DIR:$PATH"
export VERSION="1.9"
export srcdir="tests"
export builddir="."
export PATH="$SCRIPT_DIR/tests:$PATH"

# Trace env vars for LD_PRELOAD are set per-test below.

################################################################
# Test 1: help-version.sh
################################################################
echo "Test 1 started"
LOG1="flow_results/test1.log"
cd "$SCRIPT_DIR/tests"
LD_PRELOAD=libtracer.so TRACE_OUTPUT=$SCRIPT_DIR/flow_results/test1_trace.log \
    ./help-version.sh > "$SCRIPT_DIR/$LOG1" 2>&1
rc1=$?
cd "$SCRIPT_DIR"
if [ $rc1 -eq 0 ]; then
    echo "Test 1 passed"
    mv "$LOG1" "flow_results/test1_success.log"
else
    echo "Test 1 failed" >&2
    mv "$LOG1" "flow_results/test1_fail.log"
    failed=1
fi
echo "Test 1 ended"

################################################################
# Test 2: time-max-rss.sh
################################################################
echo "Test 2 started"
LOG2="flow_results/test2.log"
cd "$SCRIPT_DIR/tests"
LD_PRELOAD=libtracer.so TRACE_OUTPUT=$SCRIPT_DIR/flow_results/test2_trace.log \
    ./time-max-rss.sh > "$SCRIPT_DIR/$LOG2" 2>&1
rc2=$?
cd "$SCRIPT_DIR"
if [ $rc2 -eq 0 ]; then
    echo "Test 2 passed"
    mv "$LOG2" "flow_results/test2_success.log"
else
    echo "Test 2 failed" >&2
    mv "$LOG2" "flow_results/test2_fail.log"
    failed=1
fi
echo "Test 2 ended"

################################################################
# Test 3: time-exit-codes.sh
################################################################
echo "Test 3 started"
LOG3="flow_results/test3.log"
cd "$SCRIPT_DIR/tests"
LD_PRELOAD=libtracer.so TRACE_OUTPUT=$SCRIPT_DIR/flow_results/test3_trace.log \
    ./time-exit-codes.sh > "$SCRIPT_DIR/$LOG3" 2>&1
rc3=$?
cd "$SCRIPT_DIR"
if [ $rc3 -eq 0 ]; then
    echo "Test 3 passed"
    mv "$LOG3" "flow_results/test3_success.log"
else
    echo "Test 3 failed" >&2
    mv "$LOG3" "flow_results/test3_fail.log"
    failed=1
fi
echo "Test 3 ended"

################################################################
# Test 4: time-posix-quiet.sh
################################################################
echo "Test 4 started"
LOG4="flow_results/test4.log"
cd "$SCRIPT_DIR/tests"
LD_PRELOAD=libtracer.so TRACE_OUTPUT=$SCRIPT_DIR/flow_results/test4_trace.log \
    ./time-posix-quiet.sh > "$SCRIPT_DIR/$LOG4" 2>&1
rc4=$?
cd "$SCRIPT_DIR"
if [ $rc4 -eq 0 ]; then
    echo "Test 4 passed"
    mv "$LOG4" "flow_results/test4_success.log"
else
    echo "Test 4 failed" >&2
    mv "$LOG4" "flow_results/test4_fail.log"
    failed=1
fi
echo "Test 4 ended"

exit $failed

