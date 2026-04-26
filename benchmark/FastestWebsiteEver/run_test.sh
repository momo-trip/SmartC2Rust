
#!/bin/bash

# Reformed test cases
failed=0

mkdir -p flow_results

cleanup_port() {
    lsof -ti :80 | xargs -r kill -9 2>/dev/null
    sleep 1
}

#############################################
# Test 1: Server startup and basic request
#############################################
echo "Test 1 started"
TEST1_LOG="flow_results/test1.log"
: > "$TEST1_LOG"

cleanup_port

cd server
LD_PRELOAD=libtracer.so TRACE_OUTPUT=$PWD/../flow_results/test1_trace.log ./cpkthttp_t1 > ../flow_results/test1_server.log 2>&1 &
SERVER_PID=$!
cd ..
sleep 2

test1_status=0
if ! ps -p $SERVER_PID > /dev/null; then
    echo "Server startup failed" >> "$TEST1_LOG"
    cat flow_results/test1_server.log >> "$TEST1_LOG"
    test1_status=1
else
    echo "Server started successfully (PID: $SERVER_PID)" >> "$TEST1_LOG"

    OUTPUT_FILE="flow_results/test1_output.bin"
    curl_output=$(curl -s -w "%{http_code}" http://localhost:80 -o "$OUTPUT_FILE")
    HTTP_CODE=${curl_output: -3}
    echo "HTTP Code: $HTTP_CODE" >> "$TEST1_LOG"

    if [ "$HTTP_CODE" != "200" ]; then
        echo "Request failed: HTTP code $HTTP_CODE" >> "$TEST1_LOG"
        test1_status=1
    else
        RESPONSE_SIZE=$(stat -c%s "$OUTPUT_FILE" 2>/dev/null)
        echo "Response size: $RESPONSE_SIZE" >> "$TEST1_LOG"
        if [ -z "$RESPONSE_SIZE" ] || [ "$RESPONSE_SIZE" -lt 10 ]; then
            echo "Abnormal response size: $RESPONSE_SIZE bytes" >> "$TEST1_LOG"
            test1_status=1
        else
            echo "Request successful: received $RESPONSE_SIZE bytes" >> "$TEST1_LOG"
        fi
    fi

    kill $SERVER_PID 2>/dev/null
    sleep 1
    kill -9 $SERVER_PID 2>/dev/null
fi

cleanup_port

if [ $test1_status -eq 0 ]; then
    echo "Test 1 passed"
    mv "$TEST1_LOG" flow_results/test1_success.log
else
    echo "Test 1 failed" >&2
    mv "$TEST1_LOG" flow_results/test1_fail.log
    failed=1
fi
echo "Test 1 ended"

#############################################
# Test 2: Multiple sequential requests
#############################################
echo "Test 2 started"
TEST2_LOG="flow_results/test2.log"
: > "$TEST2_LOG"

cleanup_port

cd server
LD_PRELOAD=libtracer.so TRACE_OUTPUT=$PWD/../flow_results/test2_trace.log ./cpkthttp_t2 > ../flow_results/test2_server.log 2>&1 &
SERVER_PID=$!
cd ..
sleep 2

test2_status=0
if ! ps -p $SERVER_PID > /dev/null; then
    echo "Server startup failed" >> "$TEST2_LOG"
    cat flow_results/test2_server.log >> "$TEST2_LOG"
    test2_status=1
else
    echo "Server started (PID: $SERVER_PID)" >> "$TEST2_LOG"
    for i in 1 2 3 4 5; do
        code=$(curl -s -o /dev/null -w "%{http_code}" http://localhost:80)
        echo "Request $i: HTTP $code" >> "$TEST2_LOG"
        if [ "$code" != "200" ]; then
            echo "Request $i failed: HTTP $code" >> "$TEST2_LOG"
            test2_status=1
            break
        fi
    done

    kill $SERVER_PID 2>/dev/null
    sleep 1
    kill -9 $SERVER_PID 2>/dev/null
fi

cleanup_port

if [ $test2_status -eq 0 ]; then
    echo "Test 2 passed"
    mv "$TEST2_LOG" flow_results/test2_success.log
else
    echo "Test 2 failed" >&2
    mv "$TEST2_LOG" flow_results/test2_fail.log
    failed=1
fi
echo "Test 2 ended"

#############################################
# Test 3: Server termination
#############################################
echo "Test 3 started"
TEST3_LOG="flow_results/test3.log"
: > "$TEST3_LOG"

cleanup_port

cd server
LD_PRELOAD=libtracer.so TRACE_OUTPUT=$PWD/../flow_results/test3_trace.log ./cpkthttp_t3 > ../flow_results/test3_server.log 2>&1 &
SERVER_PID=$!
cd ..
sleep 2

test3_status=0
if ! ps -p $SERVER_PID > /dev/null; then
    echo "Server startup failed" >> "$TEST3_LOG"
    cat flow_results/test3_server.log >> "$TEST3_LOG"
    test3_status=1
else
    echo "Server started (PID: $SERVER_PID)" >> "$TEST3_LOG"

    # Do a request to confirm server is responsive
    code=$(curl -s -o /dev/null -w "%{http_code}" http://localhost:80)
    echo "Initial request HTTP code: $code" >> "$TEST3_LOG"
    if [ "$code" != "200" ]; then
        echo "Initial request failed" >> "$TEST3_LOG"
        test3_status=1
    fi

    kill $SERVER_PID
    sleep 2

    if ps -p $SERVER_PID > /dev/null; then
        echo "Server did not terminate with SIGTERM, forcing" >> "$TEST3_LOG"
        kill -9 $SERVER_PID
        sleep 1
        if ps -p $SERVER_PID > /dev/null; then
            echo "Server termination failed" >> "$TEST3_LOG"
            test3_status=1
        else
            echo "Server force-terminated" >> "$TEST3_LOG"
        fi
    else
        echo "Server terminated normally" >> "$TEST3_LOG"
    fi
fi

cleanup_port

if [ $test3_status -eq 0 ]; then
    echo "Test 3 passed"
    mv "$TEST3_LOG" flow_results/test3_success.log
else
    echo "Test 3 failed" >&2
    mv "$TEST3_LOG" flow_results/test3_fail.log
    failed=1
fi
echo "Test 3 ended"

exit $failed

