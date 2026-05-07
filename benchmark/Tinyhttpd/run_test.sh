
#!/bin/bash

# Reformed test cases

failed=0
mkdir -p flow_results

cleanup_port() {
    lsof -ti :4000 | xargs -r kill -9 2>/dev/null
    sleep 1
    pkill -f "./test_httpd_t1" 2>/dev/null
    pkill -f "./test_httpd_t2" 2>/dev/null
    pkill -f "./test_httpd_t3" 2>/dev/null
    pkill -f "./test_client_t3" 2>/dev/null
    sleep 1
}

###############################################
# Test 1: Test if HTTP server can start
###############################################
echo "Test 1 started"
cleanup_port

LD_PRELOAD=libtracer.so TRACE_OUTPUT=$PWD/flow_results/test1_trace.log ./test_httpd_t1 &
server_pid=$!

sleep 3

if ps -p $server_pid > /dev/null; then
    echo "Test 1 passed"
    echo "Server PID: $server_pid" > flow_results/test1_success.log
else
    echo "Test 1 failed" >&2
    echo "Failed to start server" > flow_results/test1_fail.log
    failed=1
fi

# Kill server for this testcase
kill -9 $server_pid 2>/dev/null
cleanup_port
echo "Test 1 ended"

###############################################
# Test 2: Test sending HTTP request with curl
###############################################
echo "Test 2 started"
cleanup_port

LD_PRELOAD=libtracer.so TRACE_OUTPUT=$PWD/flow_results/test2_trace.log ./test_httpd_t2 &
server_pid=$!
sleep 3

response=$(curl -s --connect-timeout 5 http://localhost:4000/)
if [[ $response == *"Welcome to J. David's webserver"* ]]; then
    echo "Test 2 passed"
    echo "$response" > flow_results/test2_success.log
else
    echo "Test 2 failed" >&2
    echo "Response: $response" > flow_results/test2_fail.log
    failed=1
fi

kill -9 $server_pid 2>/dev/null
cleanup_port
echo "Test 2 ended"

###############################################
# Test 3: Test client communication with server
###############################################
echo "Test 3 started"
cleanup_port

LD_PRELOAD=libtracer.so TRACE_OUTPUT=$PWD/flow_results/test3_httpd_trace.log ./test_httpd_t3 &
server_pid=$!
sleep 3

if [ ! -f "./test_client_t3" ]; then
    echo "Test 3 failed" >&2
    echo "Client executable not found" > flow_results/test3_fail.log
    failed=1
else
    client_output=$(LD_PRELOAD=libtracer.so TRACE_OUTPUT=$PWD/flow_results/test3_trace.log timeout 10 ./test_client_t3 2>&1)
    client_exit_code=$?

    if [ $client_exit_code -eq 0 ]; then
        if [[ $client_output == *"char from server = H"* ]]; then
            echo "Test 3 passed"
            echo "$client_output" > flow_results/test3_success.log
        else
            echo "Test 3 failed" >&2
            echo "Actual: $client_output" > flow_results/test3_fail.log
            failed=1
        fi
    else
        echo "Test 3 failed" >&2
        echo "Client output: $client_output" > flow_results/test3_fail.log
        failed=1
    fi
fi

kill -9 $server_pid 2>/dev/null
cleanup_port
echo "Test 3 ended"

exit $failed

