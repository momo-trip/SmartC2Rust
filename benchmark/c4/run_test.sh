
#!/bin/bash

# Reformed test cases

failed=0

# Directory settings
EXPECTED_DIR="./expected"
RESULTS_DIR="./test-results"
TEMP_DIR="./test-temp"
FLOW_DIR="./flow_results"

HELLO_C="hello.c"
ARGINC_C="arginc.c"

mkdir -p "$RESULTS_DIR"
mkdir -p "$TEMP_DIR"
mkdir -p "$FLOW_DIR"

normalize() {
    local input_file="$1"
    local output_file="$2"
    cat "$input_file" | \
        sed 's/IMM  [-0-9][0-9]*/IMM  XXXX/g' | \
        sed 's/cycle = [0-9][0-9]*/cycle = XXXX/g' | \
        sed 's/LEA  [0-9][0-9]*/LEA  XXXX/g' | \
        sed 's/exit([0-9][0-9]*)/exit(XXXX)/g' > "$output_file"
}

# -------------------- Test 1: Compile and execute hello.c --------------------
test_num=1
echo "Test ${test_num} started"
LOG_FILE="$FLOW_DIR/test${test_num}_run.log"
temp_file="$TEMP_DIR/test${test_num}_raw.txt"

LD_PRELOAD=libtracer.so TRACE_OUTPUT=$PWD/flow_results/test${test_num}_trace.log ./test_c4_t1 $HELLO_C > "$temp_file" 2>&1
rc=$?
cp "$temp_file" "$LOG_FILE"

test1_pass=0
if [ $rc -eq 0 ]; then
    normalize "$temp_file" "$RESULTS_DIR/test${test_num}_result.txt"
    if diff -q "$RESULTS_DIR/test${test_num}_result.txt" "$EXPECTED_DIR/test${test_num}_result.txt" >/dev/null; then
        test1_pass=1
    fi
fi

if [ $test1_pass -eq 1 ]; then
    echo "Test ${test_num} passed"
    mv "$LOG_FILE" "$FLOW_DIR/test${test_num}_success.log"
else
    echo "Test ${test_num} failed" >&2
    mv "$LOG_FILE" "$FLOW_DIR/test${test_num}_fail.log"
    failed=1
fi
echo "Test ${test_num} ended"

# -------------------- Test 2: Intermediate code for hello.c --------------------
test_num=2
echo "Test ${test_num} started"
LOG_FILE="$FLOW_DIR/test${test_num}_run.log"
temp_file="$TEMP_DIR/test${test_num}_raw.txt"

LD_PRELOAD=libtracer.so TRACE_OUTPUT=$PWD/flow_results/test${test_num}_trace.log ./test_c4_t2 -s $HELLO_C > "$temp_file" 2>&1
rc=$?
cp "$temp_file" "$LOG_FILE"

test2_pass=0
if [ $rc -eq 0 ]; then
    normalize "$temp_file" "$RESULTS_DIR/test${test_num}_result.txt"
    if diff -q "$RESULTS_DIR/test${test_num}_result.txt" "$EXPECTED_DIR/test${test_num}_result.txt" >/dev/null; then
        test2_pass=1
    fi
fi

if [ $test2_pass -eq 1 ]; then
    echo "Test ${test_num} passed"
    mv "$LOG_FILE" "$FLOW_DIR/test${test_num}_success.log"
else
    echo "Test ${test_num} failed" >&2
    mv "$LOG_FILE" "$FLOW_DIR/test${test_num}_fail.log"
    failed=1
fi
echo "Test ${test_num} ended"

# -------------------- Test 3: Compile and execute arginc.c --------------------
test_num=3
echo "Test ${test_num} started"
LOG_FILE="$FLOW_DIR/test${test_num}_run.log"
temp_file="$TEMP_DIR/test${test_num}_raw.txt"

LD_PRELOAD=libtracer.so TRACE_OUTPUT=$PWD/flow_results/test${test_num}_trace.log ./test_c4_t3 $ARGINC_C > "$temp_file" 2>&1
rc=$?
cp "$temp_file" "$LOG_FILE"

normalize "$temp_file" "$RESULTS_DIR/test${test_num}_result.txt"

test3_pass=0
expected_exit=0
if [ -f "$EXPECTED_DIR/test${test_num}_exit_code.txt" ]; then
    expected_exit=$(cat "$EXPECTED_DIR/test${test_num}_exit_code.txt")
fi

if [ "$rc" = "$expected_exit" ]; then
    if diff -q "$RESULTS_DIR/test${test_num}_result.txt" "$EXPECTED_DIR/test${test_num}_result.txt" >/dev/null; then
        test3_pass=1
    fi
fi

if [ $test3_pass -eq 1 ]; then
    echo "Test ${test_num} passed"
    mv "$LOG_FILE" "$FLOW_DIR/test${test_num}_success.log"
else
    echo "Test ${test_num} failed" >&2
    mv "$LOG_FILE" "$FLOW_DIR/test${test_num}_fail.log"
    failed=1
fi
echo "Test ${test_num} ended"

# -------------------- Test 4: Intermediate code for arginc.c --------------------
test_num=4
echo "Test ${test_num} started"
LOG_FILE="$FLOW_DIR/test${test_num}_run.log"
temp_file="$TEMP_DIR/test${test_num}_raw.txt"

LD_PRELOAD=libtracer.so TRACE_OUTPUT=$PWD/flow_results/test${test_num}_trace.log ./test_c4_t4 -s $ARGINC_C > "$temp_file" 2>&1
rc=$?
cp "$temp_file" "$LOG_FILE"

test4_pass=0
if [ $rc -eq 0 ]; then
    normalize "$temp_file" "$RESULTS_DIR/test${test_num}_result.txt"
    if diff -q "$RESULTS_DIR/test${test_num}_result.txt" "$EXPECTED_DIR/test${test_num}_result.txt" >/dev/null; then
        test4_pass=1
    fi
fi

if [ $test4_pass -eq 1 ]; then
    echo "Test ${test_num} passed"
    mv "$LOG_FILE" "$FLOW_DIR/test${test_num}_success.log"
else
    echo "Test ${test_num} failed" >&2
    mv "$LOG_FILE" "$FLOW_DIR/test${test_num}_fail.log"
    failed=1
fi
echo "Test ${test_num} ended"

exit $failed

