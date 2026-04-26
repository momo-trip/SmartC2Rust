
#!/bin/bash

# Reformed test cases

failed=0
mkdir -p flow_results

HTTP_URL="http://user:pass@subdomain.host.com:8080/p/a/t/h?query=string#hash"
GIT_URL="git://git@github.com:jwerle/url.h.git"

# run_exists_test <test_num> <url> <component> <expected>
run_exists_test() {
    local test_num=$1
    local url=$2
    local component=$3
    local expected=$4
    local binary="./url-test-t${test_num}"
    local log_file="flow_results/test${test_num}_success.log"
    local fail_log="flow_results/test${test_num}_fail.log"

    echo "Test ${test_num} started"

    if [ ! -x "$binary" ]; then
        echo "Binary $binary not found" >&2
        echo "Binary $binary not found" > "$fail_log"
        echo "Test ${test_num} failed" >&2
        failed=1
        echo "Test ${test_num} ended"
        return
    fi

    local output
    output=$(LD_PRELOAD=libtracer.so TRACE_OUTPUT=$PWD/flow_results/test${test_num}_trace.log "$binary" "$url")
    local exit_code=$?
    local actual
    actual=$(echo "$output" | grep "EXISTS $component:" | cut -d':' -f2- | tr -d ' ')

    local content="Test #${test_num}: Existence of ${component}
Command: $binary \"$url\"
Expected: ${expected}
Actual: ${actual}
Exit code: ${exit_code}"

    if [ $exit_code -eq 0 ] && [ "$actual" = "$expected" ]; then
        echo "$content" > "$log_file"
        echo "Test ${test_num} passed"
    else
        echo "$content" > "$fail_log"
        echo "Test ${test_num} failed" >&2
        failed=1
    fi
    echo "Test ${test_num} ended"
}

# run_value_test <test_num> <url> <component> <expected>
run_value_test() {
    local test_num=$1
    local url=$2
    local component=$3
    local expected=$4
    local binary="./url-test-t${test_num}"
    local log_file="flow_results/test${test_num}_success.log"
    local fail_log="flow_results/test${test_num}_fail.log"

    echo "Test ${test_num} started"

    if [ ! -x "$binary" ]; then
        echo "Binary $binary not found" >&2
        echo "Binary $binary not found" > "$fail_log"
        echo "Test ${test_num} failed" >&2
        failed=1
        echo "Test ${test_num} ended"
        return
    fi

    local output
    output=$(LD_PRELOAD=libtracer.so TRACE_OUTPUT=$PWD/flow_results/test${test_num}_trace.log "$binary" "$url")
    local exit_code=$?
    local actual
    actual=$(echo "$output" | grep "VALUE $component:" | cut -d':' -f2- | tr -d ' ')

    local content="Test #${test_num}: Value of ${component}
Command: $binary \"$url\"
Expected: ${expected}
Actual: ${actual}
Exit code: ${exit_code}"

    if [ "$actual" = "$expected" ]; then
        echo "$content" > "$log_file"
        echo "Test ${test_num} passed"
    else
        echo "$content" > "$fail_log"
        echo "Test ${test_num} failed" >&2
        failed=1
    fi
    echo "Test ${test_num} ended"
}

# run_protocol_test <test_num> <protocol> <expected>
run_protocol_test() {
    local test_num=$1
    local protocol=$2
    local expected=$3
    local binary="./url-test-t${test_num}"
    local log_file="flow_results/test${test_num}_success.log"
    local fail_log="flow_results/test${test_num}_fail.log"

    echo "Test ${test_num} started"

    if [ ! -x "$binary" ]; then
        echo "Binary $binary not found" >&2
        echo "Binary $binary not found" > "$fail_log"
        echo "Test ${test_num} failed" >&2
        failed=1
        echo "Test ${test_num} ended"
        return
    fi

    local output
    output=$(LD_PRELOAD=libtracer.so TRACE_OUTPUT=$PWD/flow_results/test${test_num}_trace.log "$binary" "CHECK_PROTOCOL:$protocol")
    local actual
    actual=$(echo "$output" | grep "PROTOCOL_VALID $protocol:" | cut -d':' -f2- | tr -d ' ')

    local content="Test #${test_num}: Protocol validation for ${protocol}
Command: $binary \"CHECK_PROTOCOL:$protocol\"
Expected: ${expected}
Actual: ${actual}"

    if [ "$actual" = "$expected" ]; then
        echo "$content" > "$log_file"
        echo "Test ${test_num} passed"
    else
        echo "$content" > "$fail_log"
        echo "Test ${test_num} failed" >&2
        failed=1
    fi
    echo "Test ${test_num} ended"
}

# HTTP URL existence tests (1-11)
run_exists_test 1 "$HTTP_URL" "href" "1"
run_exists_test 2 "$HTTP_URL" "auth" "1"
run_exists_test 3 "$HTTP_URL" "protocol" "1"
run_exists_test 4 "$HTTP_URL" "port" "1"
run_exists_test 5 "$HTTP_URL" "hostname" "1"
run_exists_test 6 "$HTTP_URL" "host" "1"
run_exists_test 7 "$HTTP_URL" "pathname" "1"
run_exists_test 8 "$HTTP_URL" "path" "1"
run_exists_test 9 "$HTTP_URL" "hash" "1"
run_exists_test 10 "$HTTP_URL" "search" "1"
run_exists_test 11 "$HTTP_URL" "query" "1"

# HTTP URL value tests (12-21)
run_value_test 12 "$HTTP_URL" "protocol" "http"
run_value_test 13 "$HTTP_URL" "auth" "user:pass"
run_value_test 14 "$HTTP_URL" "hostname" "subdomain.host.com:8080"
run_value_test 15 "$HTTP_URL" "host" "subdomain.host.com"
run_value_test 16 "$HTTP_URL" "pathname" "/p/a/t/h"
run_value_test 17 "$HTTP_URL" "path" "/p/a/t/h?query=string#hash"
run_value_test 18 "$HTTP_URL" "search" "?query=string"
run_value_test 19 "$HTTP_URL" "query" "query=string"
run_value_test 20 "$HTTP_URL" "hash" "#hash"
run_value_test 21 "$HTTP_URL" "port" "8080"

# Git URL existence tests (22-28)
run_exists_test 22 "$GIT_URL" "href" "1"
run_exists_test 23 "$GIT_URL" "protocol" "1"
run_exists_test 24 "$GIT_URL" "host" "1"
run_exists_test 25 "$GIT_URL" "auth" "1"
run_exists_test 26 "$GIT_URL" "hostname" "1"
run_exists_test 27 "$GIT_URL" "pathname" "1"
run_exists_test 28 "$GIT_URL" "path" "1"

# Git URL value tests (29-34)
run_value_test 29 "$GIT_URL" "protocol" "git"
run_value_test 30 "$GIT_URL" "host" "github.com"
run_value_test 31 "$GIT_URL" "hostname" "github.com"
run_value_test 32 "$GIT_URL" "auth" "git"
run_value_test 33 "$GIT_URL" "pathname" "jwerle/url.h.git"
run_value_test 34 "$GIT_URL" "path" "jwerle/url.h.git"

# Protocol validation tests (35-41)
run_protocol_test 35 "http" ""
run_protocol_test 36 "https" ""
run_protocol_test 37 "git" ""
run_protocol_test 38 "ssh" ""
run_protocol_test 39 "sftp" ""
run_protocol_test 40 "ftp" ""
run_protocol_test 41 "javascript" ""

exit $failed

