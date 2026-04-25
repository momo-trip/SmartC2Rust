
#!/bin/bash

# Reformed test cases
failed=0

mkdir -p flow_results

HTTP_URL="http://user:pass@subdomain.host.com:8080/p/a/t/h?query=string#hash"
GIT_URL="git://git@github.com:jwerle/url.h.git"

run_exists_test() {
    local test_num=$1
    local binary=$2
    local url=$3
    local component=$4
    local expected=$5

    echo "Test ${test_num} started"
    local log_tmp="flow_results/test${test_num}_tmp.log"

    LD_PRELOAD=libtracer.so TRACE_OUTPUT=$PWD/flow_results/test${test_num}_trace.log ./${binary} "$url" > "$log_tmp" 2>&1
    local exit_code=$?

    local output=$(grep "EXISTS $component:" "$log_tmp" | cut -d':' -f2- | tr -d ' ')

    local log_content="Test #${test_num}: Testing existence of $component
Command: ./${binary} \"$url\"
Expected output: ${expected}
Actual output: ${output}
Exit code: ${exit_code}"

    if [ $exit_code -eq 0 ] && [ "$output" = "$expected" ]; then
        echo "$log_content" > "flow_results/test${test_num}_success.log"
        echo "Test ${test_num} passed"
    else
        echo "$log_content" > "flow_results/test${test_num}_fail.log"
        echo "Test ${test_num} failed" >&2
        failed=1
    fi
    rm -f "$log_tmp"
    echo "Test ${test_num} ended"
}

run_value_test() {
    local test_num=$1
    local binary=$2
    local url=$3
    local component=$4
    local expected=$5

    echo "Test ${test_num} started"
    local log_tmp="flow_results/test${test_num}_tmp.log"

    LD_PRELOAD=libtracer.so TRACE_OUTPUT=$PWD/flow_results/test${test_num}_trace.log ./${binary} "$url" > "$log_tmp" 2>&1
    local exit_code=$?

    local output=$(grep "VALUE $component:" "$log_tmp" | cut -d':' -f2- | tr -d ' ')

    local log_content="Test #${test_num}: Testing value of $component
Command: ./${binary} \"$url\"
Expected output: ${expected}
Actual output: ${output}
Exit code: ${exit_code}"

    if [ "$output" = "$expected" ]; then
        echo "$log_content" > "flow_results/test${test_num}_success.log"
        echo "Test ${test_num} passed"
    else
        echo "$log_content" > "flow_results/test${test_num}_fail.log"
        echo "Test ${test_num} failed" >&2
        failed=1
    fi
    rm -f "$log_tmp"
    echo "Test ${test_num} ended"
}

run_protocol_test() {
    local test_num=$1
    local binary=$2
    local protocol=$3
    local expected=$4

    echo "Test ${test_num} started"
    local log_tmp="flow_results/test${test_num}_tmp.log"

    LD_PRELOAD=libtracer.so TRACE_OUTPUT=$PWD/flow_results/test${test_num}_trace.log ./${binary} "CHECK_PROTOCOL:$protocol" > "$log_tmp" 2>&1
    local exit_code=$?

    local output=$(grep "PROTOCOL_VALID $protocol:" "$log_tmp" | cut -d':' -f2- | tr -d ' ')

    local log_content="Test #${test_num}: Testing protocol validation for $protocol
Command: ./${binary} \"CHECK_PROTOCOL:$protocol\"
Expected output: ${expected}
Actual output: ${output}
Exit code: ${exit_code}"

    if [ "$output" = "$expected" ]; then
        echo "$log_content" > "flow_results/test${test_num}_success.log"
        echo "Test ${test_num} passed"
    else
        echo "$log_content" > "flow_results/test${test_num}_fail.log"
        echo "Test ${test_num} failed" >&2
        failed=1
    fi
    rm -f "$log_tmp"
    echo "Test ${test_num} ended"
}

# HTTP URL existence tests (1-11)
run_exists_test  1 url-test_t1  "$HTTP_URL" "href"     "1"
run_exists_test  2 url-test_t2  "$HTTP_URL" "auth"     "1"
run_exists_test  3 url-test_t3  "$HTTP_URL" "protocol" "1"
run_exists_test  4 url-test_t4  "$HTTP_URL" "port"     "1"
run_exists_test  5 url-test_t5  "$HTTP_URL" "hostname" "1"
run_exists_test  6 url-test_t6  "$HTTP_URL" "host"     "1"
run_exists_test  7 url-test_t7  "$HTTP_URL" "pathname" "1"
run_exists_test  8 url-test_t8  "$HTTP_URL" "path"     "1"
run_exists_test  9 url-test_t9  "$HTTP_URL" "hash"     "1"
run_exists_test 10 url-test_t10 "$HTTP_URL" "search"   "1"
run_exists_test 11 url-test_t11 "$HTTP_URL" "query"    "1"

# HTTP URL value tests (12-21)
run_value_test 12 url-test_t12 "$HTTP_URL" "protocol" "http"
run_value_test 13 url-test_t13 "$HTTP_URL" "auth"     "user:pass"
run_value_test 14 url-test_t14 "$HTTP_URL" "hostname" "subdomain.host.com:8080"
run_value_test 15 url-test_t15 "$HTTP_URL" "host"     "subdomain.host.com"
run_value_test 16 url-test_t16 "$HTTP_URL" "pathname" "/p/a/t/h"
run_value_test 17 url-test_t17 "$HTTP_URL" "path"     "/p/a/t/h?query=string#hash"
run_value_test 18 url-test_t18 "$HTTP_URL" "search"   "?query=string"
run_value_test 19 url-test_t19 "$HTTP_URL" "query"    "query=string"
run_value_test 20 url-test_t20 "$HTTP_URL" "hash"     "#hash"
run_value_test 21 url-test_t21 "$HTTP_URL" "port"     "8080"

# Git URL existence tests (22-28)
run_exists_test 22 url-test_t22 "$GIT_URL" "href"     "1"
run_exists_test 23 url-test_t23 "$GIT_URL" "protocol" "1"
run_exists_test 24 url-test_t24 "$GIT_URL" "host"     "1"
run_exists_test 25 url-test_t25 "$GIT_URL" "auth"     "1"
run_exists_test 26 url-test_t26 "$GIT_URL" "hostname" "1"
run_exists_test 27 url-test_t27 "$GIT_URL" "pathname" "1"
run_exists_test 28 url-test_t28 "$GIT_URL" "path"     "1"

# Git URL value tests (29-34)
run_value_test 29 url-test_t29 "$GIT_URL" "protocol" "git"
run_value_test 30 url-test_t30 "$GIT_URL" "host"     "github.com"
run_value_test 31 url-test_t31 "$GIT_URL" "hostname" "github.com"
run_value_test 32 url-test_t32 "$GIT_URL" "auth"     "git"
run_value_test 33 url-test_t33 "$GIT_URL" "pathname" "jwerle/url.h.git"
run_value_test 34 url-test_t34 "$GIT_URL" "path"     "jwerle/url.h.git"

# Protocol validation tests (35-41)
run_protocol_test 35 url-test_t35 "http"       ""
run_protocol_test 36 url-test_t36 "https"      ""
run_protocol_test 37 url-test_t37 "git"        ""
run_protocol_test 38 url-test_t38 "ssh"        ""
run_protocol_test 39 url-test_t39 "sftp"       ""
run_protocol_test 40 url-test_t40 "ftp"        ""
run_protocol_test 41 url-test_t41 "javascript" ""

exit $failed

