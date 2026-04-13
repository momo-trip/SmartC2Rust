
#!/bin/bash

# Reformed test cases
failed=0

mkdir -p genifai_results

# Helper: cleanup function for mock server
cleanup_mock() {
    if [ -n "$MOCK_PID" ] && kill -0 "$MOCK_PID" 2>/dev/null; then
        kill "$MOCK_PID" 2>/dev/null
        wait "$MOCK_PID" 2>/dev/null
    fi
    MOCK_PID=""
}

# Python mock RCON server script
MOCK_SERVER_SCRIPT=$(cat << 'PYEOF'
import socket
import struct
import sys
import threading
import time

HOST = "127.0.0.1"
PORT = int(sys.argv[1]) if len(sys.argv) > 1 else 25575
PASSWORD = sys.argv[2] if len(sys.argv) > 2 else "test"

def handle_client(conn, addr):
    try:
        while True:
            header = b""
            while len(header) < 4:
                chunk = conn.recv(4 - len(header))
                if not chunk:
                    return
                header += chunk
            length = struct.unpack("<i", header)[0]
            payload = b""
            while len(payload) < length:
                chunk = conn.recv(length - len(payload))
                if not chunk:
                    return
                payload += chunk
            request_id = struct.unpack("<i", payload[0:4])[0]
            ptype = struct.unpack("<i", payload[4:8])[0]
            body = payload[8:-2].decode("utf-8", errors="replace")

            if ptype == 3:  # Login
                if body == PASSWORD:
                    resp_id = request_id
                else:
                    resp_id = -1
                resp_body = b""
                resp_type = 2
                resp_payload = struct.pack("<ii", resp_id, resp_type) + resp_body + b"\x00\x00"
                resp_header = struct.pack("<i", len(resp_payload))
                conn.sendall(resp_header + resp_payload)
            elif ptype == 2:  # Command
                response_text = "Command received: " + body
                resp_body = response_text.encode("utf-8")
                resp_payload = struct.pack("<ii", request_id, 0) + resp_body + b"\x00\x00"
                resp_header = struct.pack("<i", len(resp_payload))
                conn.sendall(resp_header + resp_payload)
            else:
                pass
    except Exception as e:
        pass
    finally:
        conn.close()

def main():
    srv = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    srv.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
    srv.bind((HOST, PORT))
    srv.listen(5)
    srv.settimeout(60)
    print(f"Mock RCON server listening on {HOST}:{PORT}", flush=True)
    try:
        while True:
            try:
                conn, addr = srv.accept()
                t = threading.Thread(target=handle_client, args=(conn, addr))
                t.daemon = True
                t.start()
            except socket.timeout:
                break
    except KeyboardInterrupt:
        pass
    finally:
        srv.close()

if __name__ == "__main__":
    main()
PYEOF
)

start_mock_server() {
    local port="$1"
    local password="$2"
    python3 -c "$MOCK_SERVER_SCRIPT" "$port" "$password" &
    MOCK_PID=$!
    sleep 1
}

# ============================================================
# Test 1: Help output (-h flag)
# ============================================================
echo "Test 1 started"
log_file="genifai_results/test1.log"
LD_PRELOAD=libtracer.so TRACE_OUTPUT=$PWD/genifai_results/test1_trace.log ./mcrcon_t1 -h > "$log_file" 2>&1
test1_exit=$?
if grep -q "Usage" "$log_file"; then
    echo "Test 1 passed"
    mv "$log_file" genifai_results/test1_success.log
else
    echo "Test 1 failed"
    echo "Test 1 failed" >&2
    mv "$log_file" genifai_results/test1_fail.log
    failed=1
fi
echo "Test 1 ended"

# ============================================================
# Test 2: Version output (-v flag)
# ============================================================
echo "Test 2 started"
log_file="genifai_results/test2.log"
LD_PRELOAD=libtracer.so TRACE_OUTPUT=$PWD/genifai_results/test2_trace.log ./mcrcon_t2 -v > "$log_file" 2>&1
test2_exit=$?
if grep -qi "mcrcon" "$log_file"; then
    echo "Test 2 passed"
    mv "$log_file" genifai_results/test2_success.log
else
    echo "Test 2 failed"
    echo "Test 2 failed" >&2
    mv "$log_file" genifai_results/test2_fail.log
    failed=1
fi
echo "Test 2 ended"

# ============================================================
# Test 3: Connection refused (no server running)
# ============================================================
echo "Test 3 started"
log_file="genifai_results/test3.log"
LD_PRELOAD=libtracer.so TRACE_OUTPUT=$PWD/genifai_results/test3_trace.log ./mcrcon_t3 -H 127.0.0.1 -P 39999 -p test "test_command" > "$log_file" 2>&1
test3_exit=$?
if [ $test3_exit -ne 0 ]; then
    echo "Test 3 passed"
    mv "$log_file" genifai_results/test3_success.log
else
    echo "Test 3 failed"
    echo "Test 3 failed" >&2
    mv "$log_file" genifai_results/test3_fail.log
    failed=1
fi
echo "Test 3 ended"

# ============================================================
# Test 4: Authentication and command with mock server
# ============================================================
echo "Test 4 started"
log_file="genifai_results/test4.log"
MOCK_PID=""
start_mock_server 30401 "testpass4"
LD_PRELOAD=libtracer.so TRACE_OUTPUT=$PWD/genifai_results/test4_trace.log ./mcrcon_t4 -H 127.0.0.1 -P 30401 -p testpass4 "hello_world" > "$log_file" 2>&1
test4_exit=$?
cleanup_mock
if grep -q "Command received: hello_world" "$log_file"; then
    echo "Test 4 passed"
    mv "$log_file" genifai_results/test4_success.log
else
    echo "Test 4 failed"
    echo "Test 4 failed" >&2
    mv "$log_file" genifai_results/test4_fail.log
    failed=1
fi
echo "Test 4 ended"

# ============================================================
# Test 5: Wrong password with mock server
# ============================================================
echo "Test 5 started"
log_file="genifai_results/test5.log"
MOCK_PID=""
start_mock_server 30402 "correctpass"
LD_PRELOAD=libtracer.so TRACE_OUTPUT=$PWD/genifai_results/test5_trace.log ./mcrcon_t5 -H 127.0.0.1 -P 30402 -p wrongpass "test_cmd" > "$log_file" 2>&1
test5_exit=$?
cleanup_mock
if [ $test5_exit -ne 0 ]; then
    echo "Test 5 passed"
    mv "$log_file" genifai_results/test5_success.log
else
    echo "Test 5 failed"
    echo "Test 5 failed" >&2
    mv "$log_file" genifai_results/test5_fail.log
    failed=1
fi
echo "Test 5 ended"

# ============================================================
# Test 6: Silent mode (-s flag) with mock server
# ============================================================
echo "Test 6 started"
log_file="genifai_results/test6.log"
MOCK_PID=""
start_mock_server 30403 "testpass6"
LD_PRELOAD=libtracer.so TRACE_OUTPUT=$PWD/genifai_results/test6_trace.log ./mcrcon_t6 -s -H 127.0.0.1 -P 30403 -p testpass6 "silent_test" > "$log_file" 2>&1
test6_exit=$?
cleanup_mock
# In silent mode, the output should be minimal or empty (no extra decoration)
if [ $test6_exit -eq 0 ]; then
    echo "Test 6 passed"
    mv "$log_file" genifai_results/test6_success.log
else
    echo "Test 6 failed"
    echo "Test 6 failed" >&2
    mv "$log_file" genifai_results/test6_fail.log
    failed=1
fi
echo "Test 6 ended"

# ============================================================
# Test 7: Multiple commands with mock server
# ============================================================
echo "Test 7 started"
log_file="genifai_results/test7.log"
MOCK_PID=""
start_mock_server 30404 "testpass7"
LD_PRELOAD=libtracer.so TRACE_OUTPUT=$PWD/genifai_results/test7_trace.log ./mcrcon_t7 -H 127.0.0.1 -P 30404 -p testpass7 "cmd1" "cmd2" > "$log_file" 2>&1
test7_exit=$?
cleanup_mock
if grep -q "Command received: cmd1" "$log_file" && grep -q "Command received: cmd2" "$log_file"; then
    echo "Test 7 passed"
    mv "$log_file" genifai_results/test7_success.log
else
    echo "Test 7 failed"
    echo "Test 7 failed" >&2
    mv "$log_file" genifai_results/test7_fail.log
    failed=1
fi
echo "Test 7 ended"

# ============================================================
# Test 8: Color mode (-c flag) with mock server
# ============================================================
echo "Test 8 started"
log_file="genifai_results/test8.log"
MOCK_PID=""
start_mock_server 30405 "testpass8"
LD_PRELOAD=libtracer.so TRACE_OUTPUT=$PWD/genifai_results/test8_trace.log ./mcrcon_t8 -c -H 127.0.0.1 -P 30405 -p testpass8 "color_test" > "$log_file" 2>&1
test8_exit=$?
cleanup_mock
if [ $test8_exit -eq 0 ]; then
    echo "Test 8 passed"
    mv "$log_file" genifai_results/test8_success.log
else
    echo "Test 8 failed"
    echo "Test 8 failed" >&2
    mv "$log_file" genifai_results/test8_fail.log
    failed=1
fi
echo "Test 8 ended"

# ============================================================
# Test 9: Wait time (-w) flag with mock server
# ============================================================
echo "Test 9 started"
log_file="genifai_results/test9.log"
MOCK_PID=""
start_mock_server 30406 "testpass9"
LD_PRELOAD=libtracer.so TRACE_OUTPUT=$PWD/genifai_results/test9_trace.log ./mcrcon_t9 -w 1 -H 127.0.0.1 -P 30406 -p testpass9 "wait_test" > "$log_file" 2>&1
test9_exit=$?
cleanup_mock
if grep -q "Command received: wait_test" "$log_file"; then
    echo "Test 9 passed"
    mv "$log_file" genifai_results/test9_success.log
else
    echo "Test 9 failed"
    echo "Test 9 failed" >&2
    mv "$log_file" genifai_results/test9_fail.log
    failed=1
fi
echo "Test 9 ended"

# ============================================================
# Test 10: Environment variables (MCRCON_HOST, MCRCON_PORT, MCRCON_PASS)
# ============================================================
echo "Test 10 started"
log_file="genifai_results/test10.log"
MOCK_PID=""
start_mock_server 30407 "envpass"
MCRCON_HOST=127.0.0.1 MCRCON_PORT=30407 MCRCON_PASS=envpass LD_PRELOAD=libtracer.so TRACE_OUTPUT=$PWD/genifai_results/test10_trace.log ./mcrcon_t10 "env_test" > "$log_file" 2>&1
test10_exit=$?
cleanup_mock
if grep -q "Command received: env_test" "$log_file"; then
    echo "Test 10 passed"
    mv "$log_file" genifai_results/test10_success.log
else
    echo "Test 10 failed"
    echo "Test 10 failed" >&2
    mv "$log_file" genifai_results/test10_fail.log
    failed=1
fi
echo "Test 10 ended"

exit $failed

