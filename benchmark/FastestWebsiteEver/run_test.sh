
#!/bin/bash
# Reformed test cases

cd "$(dirname "$0")"

failed=0
mkdir -p flow_results

# Choose an available free TCP port (avoid port 80 conflicts with external services).
pick_free_port() {
    local port
    for port in 18080 18081 18082 18083 18084 18085 18090 18091 18092 18093; do
        if ! ss -ltn 2>/dev/null | awk '{print $4}' | grep -Eq "(^|:)${port}$"; then
            if [ -z "$(lsof -ti :${port} 2>/dev/null)" ]; then
                echo "$port"
                return 0
            fi
        fi
    done
    echo "18080"
    return 0
}

cleanup_port() {
    local port=$1
    local retries=0
    while [ $retries -lt 15 ]; do
        local pids
        pids=$(lsof -ti :${port} 2>/dev/null)
        if [ -z "$pids" ]; then
            if ! ss -ltn 2>/dev/null | awk '{print $4}' | grep -Eq "(^|:)${port}$"; then
                return 0
            fi
        else
            for pid in $pids; do
                kill -9 "$pid" 2>/dev/null
            done
        fi
        sleep 1
        retries=$((retries + 1))
    done
    return 0
}

wait_for_server() {
    # $1 = pid, $2 = max seconds, $3 = port
    local pid=$1
    local max=$2
    local port=$3
    local elapsed=0
    while [ $elapsed -lt $max ]; do
        if ! ps -p $pid > /dev/null 2>&1; then
            return 1
        fi
        local code
        code=$(curl -s -o /dev/null -w "%{http_code}" --max-time 1 http://localhost:${port} 2>/dev/null)
        if [ "$code" = "200" ]; then
            return 0
        fi
        sleep 1
        elapsed=$((elapsed + 1))
    done
    return 1
}

stop_server() {
    local pid=$1
    if [ -z "$pid" ]; then
        return
    fi
    kill $pid 2>/dev/null
    sleep 1
    if ps -p $pid > /dev/null 2>&1; then
        kill -9 $pid 2>/dev/null
    fi
    local retries=0
    while ps -p $pid > /dev/null 2>&1 && [ $retries -lt 10 ]; do
        sleep 1
        retries=$((retries + 1))
    done
}

# Build a per-testcase binary that listens on a chosen port by patching PORT in main.c
build_binary_with_port() {
    local binname=$1
    local port=$2
    local srcfile="server/c/main_${binname}.c"
    sed -E "s/#define[[:space:]]+PORT[[:space:]]+\"[0-9]+\"/#define PORT \"${port}\"/" server/c/main.c > "$srcfile"
    if [ ! -f "server/${binname}" ]; then
        clang -O2 -finstrument-functions -g -gdwarf-4 -o "server/${binname}" "$srcfile"
        return $?
    fi
    return 0
}

run_server_test() {
    # $1 = test num, $2 = binary name, $3 = port
    local tn=$1
    local binname=$2
    local port=$3
    local log_file="flow_results/test${tn}_tmp.log"
    : > "$log_file"

    cleanup_port $port

    cd server
    LD_PRELOAD=libtracer.so TRACE_OUTPUT=$PWD/../flow_results/test${tn}_trace.log ./${binname} > server_log_t${tn}.txt 2>&1 &
    local spid=$!
    cd ..

    wait_for_server $spid 15 $port
    local waitret=$?

    if ! ps -p $spid > /dev/null 2>&1 || [ $waitret -ne 0 ]; then
        echo "Server startup failed" >> "$log_file"
        cat server/server_log_t${tn}.txt >> "$log_file" 2>/dev/null
        stop_server $spid
        rm -f server/server_log_t${tn}.txt
        SERVER_PID=""
        LOG_FILE_RET="$log_file"
        return 1
    fi

    echo "Server started (PID: $spid) on port $port" >> "$log_file"
    SERVER_PID=$spid
    LOG_FILE_RET="$log_file"
    return 0
}

# Prepare per-test binaries each listening on a unique high port
PORT1=$(pick_free_port)
build_binary_with_port cpkthttp_t1 $PORT1

PORT2=18081
while ss -ltn 2>/dev/null | awk '{print $4}' | grep -Eq "(^|:)${PORT2}$" || [ "$PORT2" = "$PORT1" ]; do
    PORT2=$((PORT2 + 1))
done
build_binary_with_port cpkthttp_t2 $PORT2

PORT3=18082
while ss -ltn 2>/dev/null | awk '{print $4}' | grep -Eq "(^|:)${PORT3}$" || [ "$PORT3" = "$PORT1" ] || [ "$PORT3" = "$PORT2" ]; do
    PORT3=$((PORT3 + 1))
done
build_binary_with_port cpkthttp_t3 $PORT3

########################################
# Test 1: Server starts, basic request returns HTTP 200 with proper content
########################################
echo "Test 1 started"
TEST_NUM=1

run_server_test $TEST_NUM cpkthttp_t1 $PORT1
start_ok=$?
LOG_FILE="$LOG_FILE_RET"
test1_ok=1

if [ $start_ok -ne 0 ]; then
    test1_ok=0
else
    OUTPUT_FILE="test_output_t1.bin"
    curl_output=$(curl -s -w "%{http_code}" --max-time 5 http://localhost:${PORT1} -o "$OUTPUT_FILE")
    HTTP_CODE=${curl_output: -3}
    echo "HTTP code: $HTTP_CODE" >> "$LOG_FILE"
    if [ "$HTTP_CODE" != "200" ]; then
        echo "HTTP code not 200: $HTTP_CODE" >> "$LOG_FILE"
        cat server/server_log_t1.txt >> "$LOG_FILE" 2>/dev/null
        test1_ok=0
    else
        RESPONSE_SIZE=$(stat -c%s "$OUTPUT_FILE" 2>/dev/null)
        echo "Response size: $RESPONSE_SIZE" >> "$LOG_FILE"
        if [ -z "$RESPONSE_SIZE" ] || [ "$RESPONSE_SIZE" -lt 100 ]; then
            echo "Abnormal response size: $RESPONSE_SIZE bytes" >> "$LOG_FILE"
            test1_ok=0
        fi
    fi
    rm -f "$OUTPUT_FILE"
    stop_server $SERVER_PID
    rm -f server/server_log_t1.txt
fi
cleanup_port $PORT1

if [ $test1_ok -eq 1 ]; then
    echo "Test 1 passed"
    mv "$LOG_FILE" "flow_results/test${TEST_NUM}_success.log"
else
    echo "Test 1 failed" >&2
    mv "$LOG_FILE" "flow_results/test${TEST_NUM}_fail.log"
    failed=1
fi
echo "Test 1 ended"

########################################
# Test 2: Multiple sequential requests succeed
########################################
echo "Test 2 started"
TEST_NUM=2

run_server_test $TEST_NUM cpkthttp_t2 $PORT2
start_ok=$?
LOG_FILE="$LOG_FILE_RET"
test2_ok=1

if [ $start_ok -ne 0 ]; then
    test2_ok=0
else
    for i in 1 2 3 4 5; do
        code=$(curl -s -o /dev/null -w "%{http_code}" --max-time 5 http://localhost:${PORT2})
        echo "Request $i: $code" >> "$LOG_FILE"
        if [ "$code" != "200" ]; then
            test2_ok=0
        fi
    done
    stop_server $SERVER_PID
    rm -f server/server_log_t2.txt
fi
cleanup_port $PORT2

if [ $test2_ok -eq 1 ]; then
    echo "Test 2 passed"
    mv "$LOG_FILE" "flow_results/test${TEST_NUM}_success.log"
else
    echo "Test 2 failed" >&2
    mv "$LOG_FILE" "flow_results/test${TEST_NUM}_fail.log"
    failed=1
fi
echo "Test 2 ended"

########################################
# Test 3: Server terminates gracefully on SIGTERM
########################################
echo "Test 3 started"
TEST_NUM=3

run_server_test $TEST_NUM cpkthttp_t3 $PORT3
start_ok=$?
LOG_FILE="$LOG_FILE_RET"
test3_ok=1

if [ $start_ok -ne 0 ]; then
    test3_ok=0
else
    code=$(curl -s -o /dev/null -w "%{http_code}" --max-time 5 http://localhost:${PORT3})
    echo "Pre-terminate request code: $code" >> "$LOG_FILE"
    if [ "$code" != "200" ]; then
        test3_ok=0
    fi
    kill $SERVER_PID
    sleep 2
    if ps -p $SERVER_PID > /dev/null 2>&1; then
        echo "Server did not terminate with SIGTERM, force killing" >> "$LOG_FILE"
        kill -9 $SERVER_PID 2>/dev/null
        test3_ok=0
    else
        echo "Server terminated normally" >> "$LOG_FILE"
    fi
    rm -f server/server_log_t3.txt
fi
cleanup_port $PORT3

if [ $test3_ok -eq 1 ]; then
    echo "Test 3 passed"
    mv "$LOG_FILE" "flow_results/test${TEST_NUM}_success.log"
else
    echo "Test 3 failed" >&2
    mv "$LOG_FILE" "flow_results/test${TEST_NUM}_fail.log"
    failed=1
fi
echo "Test 3 ended"

exit $failed

