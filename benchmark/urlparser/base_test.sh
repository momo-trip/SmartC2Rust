#!/bin/bash

# Initialize variables
total_tests=0
passed_tests=0

# Test URLs
HTTP_URL="http://user:pass@subdomain.host.com:8080/p/a/t/h?query=string#hash"
GIT_URL="git://git@github.com:jwerle/url.h.git"

# Color definitions
GREEN='\033[0;32m'
RED='\033[0;31m'
NC='\033[0m'

# Executable name
EXECUTABLE="./url-test"

# Create results directory if it doesn't exist
rm -rf results
mkdir -p results

# Function to write test start log
log_test_start() {
    local current_test=$1
    local test_name=$2
    
    echo -e "\nTest #${current_test}: Running ${test_name}"
    echo "Test Case #$current_test: Started" >> /root/SmartC2Rust/benchmark/urlparser/out_flow_c.log
    echo "Test Case #$current_test: Started" >> /root/SmartC2Rust/benchmark/urlparser/out_flow_rust.log
}

# Function to write log
write_log() {
    local test_num=$1
    local test_name=$2
    local status=$3
    local command=$4
    local expected=$5
    local actual=$6
    local exit_code=$7

    local log_content="Test #${test_num}: ${test_name}
Command: ${command}
Expected output: ${expected}
Actual output: ${actual}
Exit code: ${exit_code}
Status: ${status}
Timestamp: $(date '+%Y-%m-%d %H:%M:%S')"

    if [ "$status" = "PASSED" ]; then
        log_file="results/test${test_num}_success.log"
    else
        log_file="results/test${test_num}_fail.log"
    fi

    echo "$log_content" > "$log_file"
}


# Test component existence
test_component_exists() {
    local url=$1
    local component=$2
    local expected=$3
    
    total_tests=$((total_tests + 1))
    local current_test=$total_tests
    
    log_test_start "$current_test" "Testing existence of $component"
    
    echo "Executed command: $EXECUTABLE \"$url\""
    
    $EXECUTABLE "$url"
    local exit_code=$?
    
    echo "Exit code: $exit_code"
    
    if [ $exit_code -ne 0 ]; then
        echo -e "Test #${current_test} failed"
        write_log "$current_test" "Testing existence of $component" "FAILED" "$EXECUTABLE \"$url\"" "$expected" "終了コード: $exit_code" "$exit_code"
        return 1
    fi
    
    local output=$($EXECUTABLE "$url" | grep "EXISTS $component:" | cut -d':' -f2- | tr -d ' ')
    
    if [ "$output" = "$expected" ]; then
        echo -e "Test #${current_test} passed"
        write_log "$current_test" "Testing existence of $component" "PASSED" "$EXECUTABLE \"$url\"" "$expected" "$output" "$exit_code"
        passed_tests=$((passed_tests + 1))
        return 0
    else
        echo -e "Test #${current_test} failed"
        echo "Expected output: '$expected'"
        echo "Actual output: '$output'"
        write_log "$current_test" "Testing existence of $component" "FAILED" "$EXECUTABLE \"$url\"" "$expected" "$output" "$exit_code"
        return 1
    fi
}

# Test component value
test_component_value() {
    local url=$1
    local component=$2
    local expected=$3
    
    total_tests=$((total_tests + 1))
    local current_test=$total_tests
    
    log_test_start "$current_test" "Testing value of $component"
    
    local command="$EXECUTABLE \"$url\" | grep \"VALUE $component:\""
    local output=$($EXECUTABLE "$url" | grep "VALUE $component:" | cut -d':' -f2- | tr -d ' ')
    
    if [ "$output" = "$expected" ]; then
        echo -e "Test #${current_test} passed"
        write_log "$current_test" "Testing value of $component" "PASSED" "$command" "$expected" "$output" "$?"
        passed_tests=$((passed_tests + 1))
        return 0
    else
        echo -e "Test #${current_test} failed"
        echo "Expected output: '$expected'"
        echo "Actual output: '$output'"
        write_log "$current_test" "Testing value of $component" "FAILED" "$command" "$expected" "$output" "$?"
        return 1
    fi
}

# Test protocol validation
test_protocol_valid() {
    local protocol=$1
    local expected=$2
    
    total_tests=$((total_tests + 1))
    local current_test=$total_tests
    
    log_test_start "$current_test" "Testing protocol validation for $protocol"
    
    local command="$EXECUTABLE \"CHECK_PROTOCOL:$protocol\""
    local output=$($EXECUTABLE "CHECK_PROTOCOL:$protocol" | grep "PROTOCOL_VALID $protocol:" | cut -d':' -f2- | tr -d ' ')
    
    if [ "$output" = "$expected" ]; then
        echo -e "Test #${current_test} passed"
        write_log "$current_test" "Testing protocol validation for $protocol" "PASSED" "$command" "$expected" "$output" "$?"
        passed_tests=$((passed_tests + 1))
        return 0
    else
        echo -e "Test #${current_test} failed"
        echo "Expected output: '$expected'"
        echo "Actual output: '$output'"
        write_log "$current_test" "Testing protocol validation for $protocol" "FAILED" "$command" "$expected" "$output" "$?"
        return 1
    fi
}

echo "=== Starting URL Parser Tests ==="

echo -e "\nTesting HTTP URL components..."

# Test HTTP URL components existence
test_component_exists "$HTTP_URL" "href" "1"
test_component_exists "$HTTP_URL" "auth" "1"
test_component_exists "$HTTP_URL" "protocol" "1"
test_component_exists "$HTTP_URL" "port" "1"
test_component_exists "$HTTP_URL" "hostname" "1"
test_component_exists "$HTTP_URL" "host" "1"
test_component_exists "$HTTP_URL" "pathname" "1"
test_component_exists "$HTTP_URL" "path" "1"
test_component_exists "$HTTP_URL" "hash" "1"
test_component_exists "$HTTP_URL" "search" "1"
test_component_exists "$HTTP_URL" "query" "1"

# Test HTTP URL component values
test_component_value "$HTTP_URL" "protocol" "http"
test_component_value "$HTTP_URL" "auth" "user:pass"
test_component_value "$HTTP_URL" "hostname" "subdomain.host.com:8080"
test_component_value "$HTTP_URL" "host" "subdomain.host.com"
test_component_value "$HTTP_URL" "pathname" "/p/a/t/h"
test_component_value "$HTTP_URL" "path" "/p/a/t/h?query=string#hash"
test_component_value "$HTTP_URL" "search" "?query=string"
test_component_value "$HTTP_URL" "query" "query=string"
test_component_value "$HTTP_URL" "hash" "#hash"
test_component_value "$HTTP_URL" "port" "8080"

echo -e "\nTesting Git URL components..."

# Test Git URL components existence
test_component_exists "$GIT_URL" "href" "1"
test_component_exists "$GIT_URL" "protocol" "1"
test_component_exists "$GIT_URL" "host" "1"
test_component_exists "$GIT_URL" "auth" "1"
test_component_exists "$GIT_URL" "hostname" "1"
test_component_exists "$GIT_URL" "pathname" "1"
test_component_exists "$GIT_URL" "path" "1"

# Test Git URL component values
test_component_value "$GIT_URL" "protocol" "git"
test_component_value "$GIT_URL" "host" "github.com"
test_component_value "$GIT_URL" "hostname" "github.com"
test_component_value "$GIT_URL" "auth" "git"
test_component_value "$GIT_URL" "pathname" "jwerle/url.h.git"
test_component_value "$GIT_URL" "path" "jwerle/url.h.git"

echo -e "\nTesting protocol validation..."

# Test protocol validation
for protocol in "http" "https" "git" "ssh" "sftp" "ftp" "javascript"; do
    test_protocol_valid "$protocol" "1"
done

# Calculate pass rate
pass_rate=$((passed_tests * 100 / total_tests))

echo -e "\n=== Test Summary ==="
echo "Total tests: $total_tests"
echo "Tests passed: $passed_tests"
echo "Tests failed: $((total_tests - passed_tests))"
echo "Pass rate: $pass_rate%"

# Check for failed tests
if [ $pass_rate -eq 100 ]; then
    echo "All tests passed successfully."
    exit 0
else
    echo "Some tests failed." >&2
    echo "Failed test cases:" >&2
    for ((i=1; i<=total_tests; i++)); do
        if [ ! -f "results/test${i}_success.log" ]; then
            echo "Test Case #$i failed" >&2
        fi
    done
    echo "Check results directory for detailed logs."
    exit 1
fi