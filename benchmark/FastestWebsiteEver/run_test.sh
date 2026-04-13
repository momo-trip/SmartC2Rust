
#!/bin/bash

# Reformed test cases for FastestWebsiteEver
failed=0

mkdir -p genifai_results

# Cleanup function to kill server processes and free port 80
cleanup_port() {
    sudo lsof -ti :80 | xargs -r sudo kill -9 2>/dev/null
    sleep 1
}

# Kill any existing cpkthttp processes
cleanup_all() {
    sudo pkill -9 -f 'cpkthttp_t' 2>/dev/null
    cleanup_port
}

# Initial cleanup
cleanup_all

###############################################################################
# Test 1: Server startup
###############################################################################
echo "Test 1 started"
test1_log=""
test1_pass=1

cd server

# Start the server in the background with tracing
test1_log+="Starting server cpkthttp_t1...\n"
LD_PRELOAD=libtracer.so TRACE_OUTPUT=$PWD/../genifai_results/test1_trace.log sudo -E ./cpkthttp_t1 > server_log.txt 2>&1 &
SERVER_PID=$!

# Wait for the server to start
sleep 2

# Check if the server started successfully
if ! ps -p $SERVER_PID > /dev/null 2>&1; then
    # The server may have been started by sudo as a child process
    # Try to find it by name
    SERVER_PID=$(pgrep -f cpkthttp_t1 | head -1)
    if [ -z "$SERVER_PID" ]; then
        test1_log+="Server startup failed\n"
        test1_log+="Server log:\n"
        test1_log+=$(cat server_log.txt 2>/dev/null)
        test1_pass=0
    fi
fi

if [ "$test1_pass" -eq 1 ]; then
    # Verify server is actually listening on port 80
    if sudo lsof -i :80 >/dev/null 2>&1; then
        test1_log+="Server started successfully (PID: $SERVER_PID)\n"
        test1_log+="Server is listening on port 80\n"
    else
        test1_log+="Server process exists but not listening on port 80\n"
        test1_pass=0
    fi
fi

cd ..

if [ "$test1_pass" -eq 1 ]; then
    echo "Test 1 passed"
    echo -e "$test1_log" > genifai_results/test1_success.log
else
    echo "Test 1 failed"
    echo "Test 1 failed" >&2
    echo -e "$test1_log" > genifai_results/test1_fail.log
    failed=1
fi
echo "Test 1 ended"

###############################################################################
# Test 2: Client request - HTTP 200 and response size >= 100 bytes
###############################################################################
echo "Test 2 started"
test2_log=""
test2_pass=1

OUTPUT_FILE="server/test_output_t2.bin"

test2_log+="Sending HTTP request to localhost:80...\n"
curl_output=$(curl -s -w "%{http_code}" http://localhost:80 -o "$OUTPUT_FILE" 2>&1)
curl_exit=$?

if [ $curl_exit -ne 0 ]; then
    test2_log+="curl command failed with exit code $curl_exit\n"
    test2_log+="curl output: $curl_output\n"
    test2_pass=0
fi

if [ "$test2_pass" -eq 1 ]; then
    HTTP_CODE=${curl_output: -3}
    test2_log+="HTTP response code: $HTTP_CODE\n"

    if [ "$HTTP_CODE" != "200" ]; then
        test2_log+="Request failed: expected HTTP 200, got $HTTP_CODE\n"
        test2_pass=0
    fi
fi

if [ "$test2_pass" -eq 1 ]; then
    RESPONSE_SIZE=$(stat -c%s "$OUTPUT_FILE" 2>/dev/null)
    test2_log+="Response size: $RESPONSE_SIZE bytes\n"

    if [ -z "$RESPONSE_SIZE" ] || [ "$RESPONSE_SIZE" -lt 100 ]; then
        test2_log+="Abnormal response size: ${RESPONSE_SIZE:-0} bytes (expected >= 100)\n"
        test2_pass=0
    else
        test2_log+="Request successful: received $RESPONSE_SIZE bytes\n"
    fi

    FILE_TYPE=$(file -b "$OUTPUT_FILE" 2>/dev/null)
    test2_log+="Received file type: $FILE_TYPE\n"
fi

rm -f "$OUTPUT_FILE"

if [ "$test2_pass" -eq 1 ]; then
    echo "Test 2 passed"
    echo -e "$test2_log" > genifai_results/test2_success.log
else
    echo "Test 2 failed"
    echo "Test 2 failed" >&2
    echo -e "$test2_log" > genifai_results/test2_fail.log
    failed=1
fi
echo "Test 2 ended"

###############################################################################
# Test 3: Multiple requests - 5 sequential requests all return 200
###############################################################################
echo "Test 3 started"
test3_log=""
test3_pass=1

test3_log+="Testing 5 sequential requests...\n"
for i in 1 2 3 4 5; do
    resp_code=$(curl -s -o /dev/null -w "%{http_code}" http://localhost:80 2>&1)
    curl_exit=$?
    test3_log+="Request $i: HTTP $resp_code (curl exit: $curl_exit)\n"

    if [ $curl_exit -ne 0 ]; then
        test3_log+="Request $i: curl failed with exit code $curl_exit\n"
        test3_pass=0
    elif [ "$resp_code" != "200" ]; then
        test3_log+="Request $i: expected HTTP 200, got $resp_code\n"
        test3_pass=0
    fi
    sleep 0.5
done

if [ "$test3_pass" -eq 1 ]; then
    echo "Test 3 passed"
    echo -e "$test3_log" > genifai_results/test3_success.log
else
    echo "Test 3 failed"
    echo "Test 3 failed" >&2
    echo -e "$test3_log" > genifai_results/test3_fail.log
    failed=1
fi
echo "Test 3 ended"

###############################################################################
# Test 4: Server termination
###############################################################################
echo "Test 4 started"
test4_log=""
test4_pass=1

# Find the server PID (it may have been started under sudo)
SERVER_PID=$(pgrep -f cpkthttp_t1 | head -1)

if [ -z "$SERVER_PID" ]; then
    test4_log+="No server process found to terminate\n"
    test4_pass=0
else
    test4_log+="Terminating server (PID: $SERVER_PID)...\n"
    sudo kill $SERVER_PID
    sleep 2

    if ps -p $SERVER_PID > /dev/null 2>&1; then
        test4_log+="Server did not terminate gracefully, force killing...\n"
        sudo kill -9 $SERVER_PID
        sleep 1
        if ps -p $SERVER_PID > /dev/null 2>&1; then
            test4_log+="Server termination failed even with SIGKILL\n"
            test4_pass=0
        else
            test4_log+="Server force terminated\n"
        fi
    else
        test4_log+="Server terminated normally\n"
    fi
fi

# Final cleanup
rm -f server/server_log.txt

if [ "$test4_pass" -eq 1 ]; then
    echo "Test 4 passed"
    echo -e "$test4_log" > genifai_results/test4_success.log
else
    echo "Test 4 failed"
    echo "Test 4 failed" >&2
    echo -e "$test4_log" > genifai_results/test4_fail.log
    failed=1
fi
echo "Test 4 ended"

exit $failed

