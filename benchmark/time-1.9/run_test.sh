
#!/bin/bash

# Reformed test cases

failed=0

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$SCRIPT_DIR" || exit 1

mkdir -p flow_results

# Check if we're in the correct directory
if [ ! -f "./time" ] || [ ! -d "./tests" ]; then
    echo "Error: Please run this script from the time_1_9 directory" >&2
    exit 1
fi

export PATH="$SCRIPT_DIR:$PATH"
export VERSION="1.9"
export srcdir="tests"
export builddir="."

TESTS_DIR="$SCRIPT_DIR/tests"
export PATH="$TESTS_DIR:$PATH"

# We'll invoke the test scripts with LD_PRELOAD on the 'time' binary via a wrapper.
# Because the actual binary under test is 'time' (and 'tests/time-aux'),
# we set LD_PRELOAD globally per-test so that traces from 'time' invocations are captured.

###############################
# Test 1: help-version.sh
###############################
test_num=1
echo "Test ${test_num} started"
LOG_FILE="$SCRIPT_DIR/flow_results/test${test_num}_tmp.log"
: > "$LOG_FILE"

cd "$TESTS_DIR" || exit 1
LD_PRELOAD=libtracer.so TRACE_OUTPUT=$SCRIPT_DIR/flow_results/test${test_num}_trace.log ./help-version.sh >> "$LOG_FILE" 2>&1
rc=$?
cd "$SCRIPT_DIR" || exit 1

if [ $rc -eq 0 ]; then
    echo "Test ${test_num} passed"
    mv "$LOG_FILE" "$SCRIPT_DIR/flow_results/test${test_num}_success.log"
else
    echo "Test ${test_num} failed" >&2
    failed=1
    mv "$LOG_FILE" "$SCRIPT_DIR/flow_results/test${test_num}_fail.log"
fi
echo "Test ${test_num} ended"

###############################
# Test 2: time-max-rss.sh
###############################
test_num=2
echo "Test ${test_num} started"
LOG_FILE="$SCRIPT_DIR/flow_results/test${test_num}_tmp.log"
: > "$LOG_FILE"

cd "$TESTS_DIR" || exit 1
LD_PRELOAD=libtracer.so TRACE_OUTPUT=$SCRIPT_DIR/flow_results/test${test_num}_trace.log ./time-max-rss.sh >> "$LOG_FILE" 2>&1
rc=$?
cd "$SCRIPT_DIR" || exit 1

if [ $rc -eq 0 ]; then
    echo "Test ${test_num} passed"
    mv "$LOG_FILE" "$SCRIPT_DIR/flow_results/test${test_num}_success.log"
else
    echo "Test ${test_num} failed" >&2
    failed=1
    mv "$LOG_FILE" "$SCRIPT_DIR/flow_results/test${test_num}_fail.log"
fi
echo "Test ${test_num} ended"

###############################
# Test 3: time-exit-codes.sh
###############################
test_num=3
echo "Test ${test_num} started"
LOG_FILE="$SCRIPT_DIR/flow_results/test${test_num}_tmp.log"
: > "$LOG_FILE"

cd "$TESTS_DIR" || exit 1
LD_PRELOAD=libtracer.so TRACE_OUTPUT=$SCRIPT_DIR/flow_results/test${test_num}_trace.log ./time-exit-codes.sh >> "$LOG_FILE" 2>&1
rc=$?
cd "$SCRIPT_DIR" || exit 1

if [ $rc -eq 0 ]; then
    echo "Test ${test_num} passed"
    mv "$LOG_FILE" "$SCRIPT_DIR/flow_results/test${test_num}_success.log"
else
    echo "Test ${test_num} failed" >&2
    failed=1
    mv "$LOG_FILE" "$SCRIPT_DIR/flow_results/test${test_num}_fail.log"
fi
echo "Test ${test_num} ended"

###############################
# Test 4: time-posix-quiet.sh
###############################
test_num=4
echo "Test ${test_num} started"
LOG_FILE="$SCRIPT_DIR/flow_results/test${test_num}_tmp.log"
: > "$LOG_FILE"

cd "$TESTS_DIR" || exit 1
LD_PRELOAD=libtracer.so TRACE_OUTPUT=$SCRIPT_DIR/flow_results/test${test_num}_trace.log ./time-posix-quiet.sh >> "$LOG_FILE" 2>&1
rc=$?
cd "$SCRIPT_DIR" || exit 1

if [ $rc -eq 0 ]; then
    echo "Test ${test_num} passed"
    mv "$LOG_FILE" "$SCRIPT_DIR/flow_results/test${test_num}_success.log"
else
    echo "Test ${test_num} failed" >&2
    failed=1
    mv "$LOG_FILE" "$SCRIPT_DIR/flow_results/test${test_num}_fail.log"
fi
echo "Test ${test_num} ended"

exit $failed

