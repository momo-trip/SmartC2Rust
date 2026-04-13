
#!/bin/bash

# Reformed test cases
failed=0

# Create results directory
rm -rf flow_results
mkdir -p flow_results

# Function to cleanup processes and ports
cleanup() {
    sudo lsof -ti :4000 | xargs -r sudo kill -9 2>/dev/null
    sleep 1
    pkill -f "./httpd_t" 2>/dev/null
    pkill -f "./client_t" 2>/dev/null
    sleep 1
}

# Set up trap to cleanup on script exit
trap cleanup EXIT

# Initial cleanup
cleanup

###############################################################################
# Test 1: Test if HTTP server can start
###############################################################################
test_num=1
echo "Test ${test_num} started"
log=""

# Start server in background
log+="Starting HTTP server...\n"
LD_PRELOAD=libtracer.so TRACE_OUTPUT=$PWD/flow_results/test${test_num}_trace.log ./httpd_t1 &
server_pid=$!

# Wait for server to start
sleep 3

# Check if server is running
if ps -p $server_pid > /dev/null 2>&1; then
    log+="HTTP server started successfully. Server PID: $server_pid\n"
    if lsof -i :4000 >/dev/null 2>&1; then
        log+="Port 4000 is successfully bound\n"
    else
        log+="Warning: Server started but port 4000 is not bound\n"
    fi
    echo -e "$log" > flow_results/test${test_num}_success.log
    echo "Test ${test_num} passed"
else
    log+="Failed to start HTTP server\n"
    echo -e "$log" > flow_results/test${test_num}_fail.log
    echo "Test ${test_num} failed"
    echo "Test ${test_num} failed" >&2
    failed=1
fi

echo "Test ${test_num} ended"

# Kill server from test 1
kill -9 $server_pid 2>/dev/null
wait $server_pid 2>/dev/null
sleep 1
cleanup

###############################################################################
# Test 2: Test sending HTTP request with curl
###############################################################################
test_num=2
echo "Test ${test_num} started"
log=""

# Start server for test 2
LD_PRELOAD=libtracer.so TRACE_OUTPUT=$PWD/flow_results/test${test_num}_trace.log ./httpd_t2 &
server_pid=$!
sleep 3

log+="Testing HTTP request...\n"
response=$(curl -s --connect-timeout 5 http://localhost:4000/ 2>&1)
curl_exit=$?
log+="Curl exit code: $curl_exit\n"
log+="Response: $response\n"

if [[ $response == *"Welcome to J. David's webserver"* ]]; then
    log+="Successfully received HTTP response\n"
    echo -e "$log" > flow_results/test${test_num}_success.log
    echo "Test ${test_num} passed"
else
    log+="Did not receive expected HTTP response\n"
    log+="Actual response: $response\n"
    echo -e "$log" > flow_results/test${test_num}_fail.log
    echo "Test ${test_num} failed"
    echo "Test ${test_num} failed" >&2
    failed=1
fi

echo "Test ${test_num} ended"

# Kill server from test 2
kill -9 $server_pid 2>/dev/null
wait $server_pid 2>/dev/null
sleep 1
cleanup

###############################################################################
# Test 3: Test client communication with server
###############################################################################
test_num=3
echo "Test ${test_num} started"
log=""

# Start server for test 3
LD_PRELOAD=libtracer.so TRACE_OUTPUT=$PWD/flow_results/test${test_num}_trace.log ./httpd_t3 &
server_pid=$!
sleep 3

log+="Testing client communication with server...\n"

if [ ! -f "./client_t3" ]; then
    log+="Client executable not found\n"
    echo -e "$log" > flow_results/test${test_num}_fail.log
    echo "Test ${test_num} failed"
    echo "Test ${test_num} failed" >&2
    failed=1
else
    log+="Running client to test communication...\n"
    client_output=$(timeout 10 ./client_t3 2>&1)
    client_exit_code=$?
    log+="Client exit code: $client_exit_code\n"
    log+="Client output: $client_output\n"

    if [ $client_exit_code -eq 0 ] && [[ $client_output == *"char from server = H"* ]]; then
        log+="Client successfully communicated with server\n"
        log+="Client received HTTP response header (H from 'HTTP/1.0')\n"
        echo -e "$log" > flow_results/test${test_num}_success.log
        echo "Test ${test_num} passed"
    else
        log+="Client did not receive expected response\n"
        log+="Expected: 'char from server = H' (first character of HTTP response)\n"
        log+="Actual output: $client_output\n"
        echo -e "$log" > flow_results/test${test_num}_fail.log
        echo "Test ${test_num} failed"
        echo "Test ${test_num} failed" >&2
        failed=1
    fi
fi

echo "Test ${test_num} ended"

# Kill server from test 3
kill -9 $server_pid 2>/dev/null
wait $server_pid 2>/dev/null

exit $failed

