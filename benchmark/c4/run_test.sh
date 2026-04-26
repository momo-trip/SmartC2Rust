
#!/bin/bash

# Reformed test cases

failed=0

EXPECTED_DIR="./expected"
RESULTS_DIR="./test-results"
TEMP_DIR="./test-temp"
FLOW_DIR="./flow_results"

HELLO_C="hello.c"
ARGINC_C="arginc.c"

mkdir -p "$RESULTS_DIR"
mkdir -p "$TEMP_DIR"
mkdir -p "$FLOW_DIR"

# Normalization function
normalize() {
    local input_file="$1"
    local output_file="$2"
    cat "$input_file" | \
        sed 's/IMM  [-0-9][0-9]*/IMM  XXXX/g' | \
        sed 's/cycle = [0-9][0-9]*/cycle = XXXX/g' | \
        sed 's/LEA  [0-9][0-9]*/LEA  XXXX/g' | \
        sed 's/exit([0-9][0-9]*)/exit(XXXX)/g' > "$output_file"
}

##############################
# Test 1: Execute hello.c
##############################
echo "Test 1 started"
log_file="$FLOW_DIR/test1_run.log"
temp_file="$TEMP_DIR/test1_raw.txt"

LD_PRELOAD=libtracer.so TRACE_OUTPUT=$PWD/flow_results/test1_trace.log ./c4_t1 $HELLO_C > "$temp_file" 2>&1
run_status=$?

{
    echo "--- Test 1: Compile and execute hello.c ---"
    echo "Exit: $run_status"
    echo "--- Raw output ---"
    cat "$temp_file"
} > "$log_file"

if [ $run_status -eq 0 ]; then
    normalize "$temp_file" "$RESULTS_DIR/test1_result.txt"
    if diff -q "$RESULTS_DIR/test1_result.txt" "$EXPECTED_DIR/test1_result.txt" >/dev/null; then
        echo "Test 1 passed"
        cat "$log_file" > "$FLOW_DIR/test1_success.log"
        rm -f "$log_file"
    else
        echo "Test 1 failed" >&2
        {
            cat "$log_file"
            echo "--- Expected ---"
            cat "$EXPECTED_DIR/test1_result.txt"
            echo "--- Actual ---"
            cat "$RESULTS_DIR/test1_result.txt"
            echo "--- Diff ---"
            diff "$EXPECTED_DIR/test1_result.txt" "$RESULTS_DIR/test1_result.txt"
        } > "$FLOW_DIR/test1_fail.log"
        rm -f "$log_file"
        failed=1
    fi
else
    echo "Test 1 failed" >&2
    cat "$log_file" > "$FLOW_DIR/test1_fail.log"
    rm -f "$log_file"
    failed=1
fi
echo "Test 1 ended"

##############################
# Test 2: Display intermediate code for hello.c
##############################
echo "Test 2 started"
log_file="$FLOW_DIR/test2_run.log"
temp_file="$TEMP_DIR/test2_raw.txt"

LD_PRELOAD=libtracer.so TRACE_OUTPUT=$PWD/flow_results/test2_trace.log ./c4_t2 -s $HELLO_C > "$temp_file" 2>&1
run_status=$?

{
    echo "--- Test 2: Display intermediate code for hello.c ---"
    echo "Exit: $run_status"
    echo "--- Raw output ---"
    cat "$temp_file"
} > "$log_file"

if [ $run_status -eq 0 ]; then
    normalize "$temp_file" "$RESULTS_DIR/test2_result.txt"
    if diff -q "$RESULTS_DIR/test2_result.txt" "$EXPECTED_DIR/test2_result.txt" >/dev/null; then
        echo "Test 2 passed"
        cat "$log_file" > "$FLOW_DIR/test2_success.log"
        rm -f "$log_file"
    else
        echo "Test 2 failed" >&2
        {
            cat "$log_file"
            echo "--- Expected ---"
            cat "$EXPECTED_DIR/test2_result.txt"
            echo "--- Actual ---"
            cat "$RESULTS_DIR/test2_result.txt"
            echo "--- Diff ---"
            diff "$EXPECTED_DIR/test2_result.txt" "$RESULTS_DIR/test2_result.txt"
        } > "$FLOW_DIR/test2_fail.log"
        rm -f "$log_file"
        failed=1
    fi
else
    echo "Test 2 failed" >&2
    cat "$log_file" > "$FLOW_DIR/test2_fail.log"
    rm -f "$log_file"
    failed=1
fi
echo "Test 2 ended"

##############################
# Test 3: Compile and execute arginc.c
##############################
echo "Test 3 started"
log_file="$FLOW_DIR/test3_run.log"
temp_file="$TEMP_DIR/test3_raw.txt"

LD_PRELOAD=libtracer.so TRACE_OUTPUT=$PWD/flow_results/test3_trace.log ./c4_t3 $ARGINC_C > "$temp_file" 2>&1
run_status=$?

{
    echo "--- Test 3: Compile and execute arginc.c ---"
    echo "Exit: $run_status"
    echo "--- Raw output ---"
    cat "$temp_file"
} > "$log_file"

normalize "$temp_file" "$RESULTS_DIR/test3_result.txt"

expected_exit=0
if [ -f "$EXPECTED_DIR/test3_exit_code.txt" ]; then
    expected_exit=$(cat "$EXPECTED_DIR/test3_exit_code.txt")
fi

if [ "$run_status" = "$expected_exit" ] && diff -q "$RESULTS_DIR/test3_result.txt" "$EXPECTED_DIR/test3_result.txt" >/dev/null; then
    echo "Test 3 passed"
    cat "$log_file" > "$FLOW_DIR/test3_success.log"
    rm -f "$log_file"
else
    echo "Test 3 failed" >&2
    {
        cat "$log_file"
        echo "--- Expected exit: $expected_exit ---"
        echo "--- Expected ---"
        cat "$EXPECTED_DIR/test3_result.txt"
        echo "--- Actual ---"
        cat "$RESULTS_DIR/test3_result.txt"
        echo "--- Diff ---"
        diff "$EXPECTED_DIR/test3_result.txt" "$RESULTS_DIR/test3_result.txt"
    } > "$FLOW_DIR/test3_fail.log"
    rm -f "$log_file"
    failed=1
fi
echo "Test 3 ended"

##############################
# Test 4: Display intermediate code for arginc.c
##############################
echo "Test 4 started"
log_file="$FLOW_DIR/test4_run.log"
temp_file="$TEMP_DIR/test4_raw.txt"

LD_PRELOAD=libtracer.so TRACE_OUTPUT=$PWD/flow_results/test4_trace.log ./c4_t4 -s $ARGINC_C > "$temp_file" 2>&1
run_status=$?

{
    echo "--- Test 4: Display intermediate code for arginc.c ---"
    echo "Exit: $run_status"
    echo "--- Raw output ---"
    cat "$temp_file"
} > "$log_file"

if [ $run_status -eq 0 ]; then
    normalize "$temp_file" "$RESULTS_DIR/test4_result.txt"
    if diff -q "$RESULTS_DIR/test4_result.txt" "$EXPECTED_DIR/test4_result.txt" >/dev/null; then
        echo "Test 4 passed"
        cat "$log_file" > "$FLOW_DIR/test4_success.log"
        rm -f "$log_file"
    else
        echo "Test 4 failed" >&2
        {
            cat "$log_file"
            echo "--- Expected ---"
            cat "$EXPECTED_DIR/test4_result.txt"
            echo "--- Actual ---"
            cat "$RESULTS_DIR/test4_result.txt"
            echo "--- Diff ---"
            diff "$EXPECTED_DIR/test4_result.txt" "$RESULTS_DIR/test4_result.txt"
        } > "$FLOW_DIR/test4_fail.log"
        rm -f "$log_file"
        failed=1
    fi
else
    echo "Test 4 failed" >&2
    cat "$log_file" > "$FLOW_DIR/test4_fail.log"
    rm -f "$log_file"
    failed=1
fi
echo "Test 4 ended"

exit $failed

