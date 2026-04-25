
#!/bin/bash

# Reformed test cases

failed=0
mkdir -p flow_results

# Cleanup function
cleanup() {
    lsof -ti :4000 2>/dev/null | xargs -r kill -9 2>/dev/null
    pkill -f "./httpd_t1" 2>/dev/null
    pkill -f "./client_t3" 2>/dev/null
    sleep 1
}

trap cleanup EXIT
cleanup

# Test 1: Test if HTTP server can start
echo "Test 1 started"
log1="flow_results/test1_trace.log"
LD_PRELOAD=libtracer.so TRACE_OUTPUT=$PWD/flow_results/test1_trace.log ./httpd_t1 > flow_results/test1_server.out 2>&1 &
server_pid=$!
sleep 3

if ps -p $server_pid > /dev/null; then
    server_running=1
else
    server_running=0
fi

if [ $server_running -eq 1 ]; then
    echo "Server PID: $server_pid" > flow_results/test1_success.log
    echo "Test 1 passed"
else
    echo "Failed to start server" > flow_results/test1_fail.log
    echo "Test 1 failed" >&2
    failed=1
fi
echo "Test 1 ended"

# Test 2: Test sending HTTP request with curl
echo "Test 2 started"
if [ $server_running -eq 1 ]; then
    response=$(curl -s --connect-timeout 5 http://localhost:4000/)
    if [[ $response == *"Welcome to J. David's webserver"* ]]; then
        echo "$response" > flow_results/test2_success.log
        echo "Test 2 passed"
    else
        echo "Response: $response" > flow_results/test2_fail.log
        echo "Test 2 failed" >&2
        failed=1
    fi
else
    echo "Server not running, skipping test" > flow_results/test2_fail.log
    echo "Test 2 failed" >&2
    failed=1
fi
echo "Test 2 ended"

# Test 3: Test client communication with server
echo "Test 3 started"
if [ ! -f "./client_t3" ]; then
    echo "Client executable not found" > flow_results/test3_fail.log
    echo "Test 3 failed" >&2
    failed=1
elif [ $server_running -ne 1 ]; then
    echo "Server not running" > flow_results/test3_fail.log
    echo "Test 3 failed" >&2
    failed=1
else
    client_output=$(timeout 10 env LD_PRELOAD=libtracer.so TRACE_OUTPUT=$PWD/flow_results/test3_trace.log ./client_t3 2>&1)
    client_exit_code=$?
    if [ $client_exit_code -eq 0 ] && [[ $client_output == *"char from server = H"* ]]; then
        echo "$client_output" > flow_results/test3_success.log
        echo "Test 3 passed"
    else
        echo "Exit code: $client_exit_code" > flow_results/test3_fail.log
        echo "Output: $client_output" >> flow_results/test3_fail.log
        echo "Test 3 failed" >&2
        failed=1
    fi
fi
echo "Test 3 ended"

exit $failed

