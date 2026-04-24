#!/bin/bash

# Initialize test counters
total_tests=3
passed_tests=0
current_test=0

# Clean up and create results directory
rm -rf results
mkdir -p results

# Function to log test start
log_test_start() {
    echo "Test Case #$1: Started" >> /root/SmartC2Rust/benchmark/Tinyhttpd/out_flow_c.log
    echo "Test Case #$1: Started" >> /root/SmartC2Rust/benchmark/Tinyhttpd/out_flow_rust.log
    echo "Test Case #$1: Started"
}

# Function to cleanup processes and ports
cleanup() {
    echo "Cleaning up processes on port 4000..."
    # More reliable cleanup
    lsof -ti :4000 | xargs -r kill -9 2>/dev/null
    sleep 1
    
    # Also ensure httpd process is terminated
    pkill -f "./httpd" 2>/dev/null
    pkill -f "./client" 2>/dev/null
    sleep 1
    
    # Double-check
    if lsof -i :4000 >/dev/null 2>&1; then
        echo "Warning: Port 4000 is still in use"
        lsof -i :4000
        echo "Please manually kill the process and try again"
        exit 1
    fi
}

# Set up trap to cleanup on script exit
trap cleanup EXIT

# Initial cleanup
cleanup

# Test 1: Test if HTTP server can start
current_test=1
log_test_start $current_test

# Start server in background
echo "Starting HTTP server..."
./httpd &
server_pid=$!

# Wait for server to start
sleep 3

# Check if server is running
if ps -p $server_pid > /dev/null; then
    echo "Test #${current_test} passed: HTTP server started successfully"
    echo "Server PID: $server_pid" > "results/test${current_test}_success.log"
    ((passed_tests++))
    
    # Also check if port is actually listening
    if lsof -i :4000 >/dev/null 2>&1; then
        echo "Port 4000 is successfully bound"
    else
        echo "Warning: Server started but port 4000 is not bound"
    fi
else
    echo "Test #${current_test} failed: Failed to start HTTP server"
    echo "Failed to start server" > "results/test${current_test}_fail.log"
    exit 1  # Cannot proceed with further tests if server doesn't start
fi

# Test 2: Test sending HTTP request with curl
current_test=2
log_test_start $current_test

echo "Testing HTTP request..."
response=$(curl -s --connect-timeout 5 http://localhost:4000/)
if [[ $response == *"Welcome to J. David's webserver"* ]]; then
    echo "Test #${current_test} passed: Successfully received HTTP response"
    echo "$response" > "results/test${current_test}_success.log"
    ((passed_tests++))
else
    echo "Test #${current_test} failed: Did not receive expected HTTP response"
    echo "Response: $response" > "results/test${current_test}_fail.log"
    echo "Actual response: $response"
fi

# Test 3: Test client communication with server
current_test=3
log_test_start $current_test

echo "Testing client communication with server..."

# Check if client executable exists
if [ ! -f "./client" ]; then
    echo "Test #${current_test} failed: Client executable not found"
    echo "Client executable not found" > "results/test${current_test}_fail.log"
else
    echo "Running client to test communication..."
    
    # Run the client and capture output
    client_output=$(timeout 10 ./client 2>&1)
    client_exit_code=$?
    
    if [ $client_exit_code -eq 0 ]; then
        echo "Client output: $client_output"
        
        # Check if client received expected response
        if [[ $client_output == *"char from server = H"* ]]; then
            echo "Test #${current_test} passed: Client successfully communicated with server"
            echo "$client_output" > "results/test${current_test}_success.log"
            ((passed_tests++))
            echo "Client received HTTP response header (H from 'HTTP/1.0')"
        else
            echo "Test #${current_test} failed: Client did not receive expected response"
            echo "Expected: 'char from server = H' (first character of HTTP response)"
            echo "Actual: $client_output" > "results/test${current_test}_fail.log"
            echo "Actual output: $client_output"
        fi
    else
        echo "Test #${current_test} failed: Client execution failed"
        echo "Client output: $client_output" > "results/test${current_test}_fail.log"
        echo "Client execution failed with exit code: $client_exit_code"
        echo "Client output: $client_output"
    fi
fi

# Calculate pass rate
pass_rate=$((passed_tests * 100 / total_tests))
echo "Test success rate: ${pass_rate}%"

# Output failed tests if any
if [ $pass_rate -ne 100 ]; then
    echo "Failed tests:" >&2
    for i in $(seq 1 $total_tests); do
        if [ -f "results/test${i}_fail.log" ]; then
            echo "Test #${i} failed" >&2
        fi
    done
    exit 1
fi

echo "All tests passed successfully!"
exit 0