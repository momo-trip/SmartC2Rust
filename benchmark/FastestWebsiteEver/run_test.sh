
#!/bin/bash
# Reformed test cases

failed=0
mkdir -p flow_results

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$SCRIPT_DIR"

cleanup_port() {
    lsof -ti :80 | xargs -r kill -9 2>/dev/null
    sleep 1
}

#############################################
# Test 1: Server startup and single request
#############################################
echo "Test 1 started"
LOG1="flow_results/test1_tmp.log"
: > "$LOG1"

cleanup_port

if pgrep cpkthttp > /dev/null; then
    echo "Warning: cpkthttp server is already running." | tee -a "$LOG1"
    echo "Test 1 failed" | tee -a "$LOG1" >&2
    mv "$LOG1" flow_results/test1_fail.log
    failed=1
else
    cd server
    LD_PRELOAD=libtracer.so TRACE_OUTPUT=$PWD/../flow_results/test1_trace.log ./cpkthttp_t1 > server_log_t1.txt 2>&1 &
    SERVER_PID=$!
    cd "$SCRIPT_DIR"
    sleep 2

    test1_pass=1
    if ! ps -p $SERVER_PID > /dev/null; then
        echo "Server startup failed" | tee -a "$LOG1"
        cat server/server_log_t1.txt >> "$LOG1" 2>/dev/null
        test1_pass=0
    else
        echo "Server started successfully (PID: $SERVER_PID)" | tee -a "$LOG1"

        OUTPUT_FILE="test_output_t1.bin"
        curl_output=$(curl -s -w "%{http_code}" http://localhost:80 -o $OUTPUT_FILE)
        HTTP_CODE=${curl_output: -3}
        echo "HTTP_CODE=$HTTP_CODE" >> "$LOG1"

        if [ "$HTTP_CODE" != "200" ]; then
            echo "Request failed: HTTP code $HTTP_CODE" | tee -a "$LOG1"
            test1_pass=0
        else
            RESPONSE_SIZE=$(stat -c%s "$OUTPUT_FILE")
            echo "RESPONSE_SIZE=$RESPONSE_SIZE" >> "$LOG1"
            if [ $RESPONSE_SIZE -lt 100 ]; then
                echo "Abnormal response size: $RESPONSE_SIZE bytes" | tee -a "$LOG1"
                test1_pass=0
            else
                echo "Request successful: received $RESPONSE_SIZE bytes" | tee -a "$LOG1"
            fi
        fi

        rm -f $OUTPUT_FILE
        kill $SERVER_PID 2>/dev/null
        sleep 1
        kill -9 $SERVER_PID 2>/dev/null
        rm -f server/server_log_t1.txt
    fi

    cleanup_port

    if [ $test1_pass -eq 1 ]; then
        echo "Test 1 passed"
        mv "$LOG1" flow_results/test1_success.log
    else
        echo "Test 1 failed" >&2
        mv "$LOG1" flow_results/test1_fail.log
        failed=1
    fi
fi
echo "Test 1 ended"

#############################################
# Test 2: File type check of response
#############################################
echo "Test 2 started"
LOG2="flow_results/test2_tmp.log"
: > "$LOG2"

cleanup_port

if pgrep cpkthttp > /dev/null; then
    echo "Warning: cpkthttp server is already running." | tee -a "$LOG2"
    echo "Test 2 failed" | tee -a "$LOG2" >&2
    mv "$LOG2" flow_results/test2_fail.log
    failed=1
else
    cd server
    LD_PRELOAD=libtracer.so TRACE_OUTPUT=$PWD/../flow_results/test2_trace.log ./cpkthttp_t2 > server_log_t2.txt 2>&1 &
    SERVER_PID=$!
    cd "$SCRIPT_DIR"
    sleep 2

    test2_pass=1
    if ! ps -p $SERVER_PID > /dev/null; then
        echo "Server startup failed" | tee -a "$LOG2"
        cat server/server_log_t2.txt >> "$LOG2" 2>/dev/null
        test2_pass=0
    else
        echo "Server started successfully (PID: $SERVER_PID)" | tee -a "$LOG2"

        OUTPUT_FILE="test_output_t2.bin"
        curl_output=$(curl -s -w "%{http_code}" http://localhost:80 -o $OUTPUT_FILE)
        HTTP_CODE=${curl_output: -3}
        echo "HTTP_CODE=$HTTP_CODE" >> "$LOG2"

        if [ "$HTTP_CODE" != "200" ]; then
            echo "Request failed: HTTP code $HTTP_CODE" | tee -a "$LOG2"
            test2_pass=0
        else
            RESPONSE_SIZE=$(stat -c%s "$OUTPUT_FILE")
            echo "RESPONSE_SIZE=$RESPONSE_SIZE" >> "$LOG2"
            if [ $RESPONSE_SIZE -lt 100 ]; then
                echo "Abnormal response size: $RESPONSE_SIZE bytes" | tee -a "$LOG2"
                test2_pass=0
            else
                echo "Request successful: received $RESPONSE_SIZE bytes" | tee -a "$LOG2"
                FILE_TYPE=$(file -b $OUTPUT_FILE)
                echo "Received file type: $FILE_TYPE" | tee -a "$LOG2"
            fi
        fi

        rm -f $OUTPUT_FILE
        kill $SERVER_PID 2>/dev/null
        sleep 1
        kill -9 $SERVER_PID 2>/dev/null
        rm -f server/server_log_t2.txt
    fi

    cleanup_port

    if [ $test2_pass -eq 1 ]; then
        echo "Test 2 passed"
        mv "$LOG2" flow_results/test2_success.log
    else
        echo "Test 2 failed" >&2
        mv "$LOG2" flow_results/test2_fail.log
        failed=1
    fi
fi
echo "Test 2 ended"

#############################################
# Test 3: Multiple requests and server termination
#############################################
echo "Test 3 started"
LOG3="flow_results/test3_tmp.log"
: > "$LOG3"

cleanup_port

if pgrep cpkthttp > /dev/null; then
    echo "Warning: cpkthttp server is already running." | tee -a "$LOG3"
    echo "Test 3 failed" | tee -a "$LOG3" >&2
    mv "$LOG3" flow_results/test3_fail.log
    failed=1
else
    cd server
    LD_PRELOAD=libtracer.so TRACE_OUTPUT=$PWD/../flow_results/test3_trace.log ./cpkthttp_t3 > server_log_t3.txt 2>&1 &
    SERVER_PID=$!
    cd "$SCRIPT_DIR"
    sleep 2

    test3_pass=1
    if ! ps -p $SERVER_PID > /dev/null; then
        echo "Server startup failed" | tee -a "$LOG3"
        cat server/server_log_t3.txt >> "$LOG3" 2>/dev/null
        test3_pass=0
    else
        echo "Server started successfully (PID: $SERVER_PID)" | tee -a "$LOG3"

        for i in 1 2 3 4 5; do
            code=$(curl -s -o /dev/null -w "%{http_code}" http://localhost:80)
            if [ "$code" != "200" ]; then
                echo "Request $i failed: HTTP $code" | tee -a "$LOG3"
                test3_pass=0
                break
            fi
            echo "Request $i: $code" | tee -a "$LOG3"
        done

        kill $SERVER_PID 2>/dev/null
        sleep 2

        if ps -p $SERVER_PID > /dev/null; then
            echo "Server termination failed" | tee -a "$LOG3"
            kill -9 $SERVER_PID 2>/dev/null
            echo "Force terminated" | tee -a "$LOG3"
            test3_pass=0
        else
            echo "Server terminated normally" | tee -a "$LOG3"
        fi
        rm -f server/server_log_t3.txt
    fi

    cleanup_port

    if [ $test3_pass -eq 1 ]; then
        echo "Test 3 passed"
        mv "$LOG3" flow_results/test3_success.log
    else
        echo "Test 3 failed" >&2
        mv "$LOG3" flow_results/test3_fail.log
        failed=1
    fi
fi
echo "Test 3 ended"

exit $failed

