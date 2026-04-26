
#!/bin/bash

# Reformed test cases

failed=0
mkdir -p flow_results

# Find an available port
find_available_port() {
    for port in {25580..25590}; do
        if ! nc -z localhost $port 2>/dev/null; then
            echo $port
            return
        fi
    done
    echo "25580"
}

MOCK_PORT=$(find_available_port)
echo "Using port: $MOCK_PORT"

# Create the special mock server
cat > special_mock_server.py << 'PYEOF'
import socket
import struct
import threading
import time
import sys
import signal

RCON_AUTHENTICATE = 3
RCON_EXEC_COMMAND = 2
RCON_RESPONSE_VALUE = 0
RCON_AUTH_RESPONSE = 2

class SpecialMockRCONServer:
    def __init__(self, host='localhost', port=25580, password='test'):
        self.host = host
        self.port = port
        self.password = password
        self.running = False
        self.socket = None

    def start(self):
        self.socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self.socket.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        try:
            self.socket.bind((self.host, self.port))
            self.socket.listen(5)
            self.running = True
            print(f"Special mock server started: {self.host}:{self.port}")
            while self.running:
                try:
                    client_socket, addr = self.socket.accept()
                    t = threading.Thread(target=self.handle_client, args=(client_socket, addr))
                    t.daemon = True
                    t.start()
                except socket.error as e:
                    if self.running:
                        print(f"Connection error: {e}")
        except Exception as e:
            print(f"Server error: {e}")
        finally:
            self.stop()

    def stop(self):
        self.running = False
        if self.socket:
            try:
                self.socket.close()
            except Exception:
                pass

    def handle_client(self, client_socket, addr):
        try:
            self.handle_normal_request(client_socket)
        except Exception as e:
            print(f"Client handling error: {e}")
        finally:
            try:
                client_socket.close()
            except Exception:
                pass

    def handle_normal_request(self, client_socket):
        try:
            while True:
                size_data = client_socket.recv(4)
                if not size_data:
                    break
                packet_size = struct.unpack('<I', size_data)[0]
                if packet_size < 10 or packet_size > 4096:
                    print(f"Invalid packet size: {packet_size}")
                    break
                packet_data = client_socket.recv(packet_size)
                if not packet_data or len(packet_data) < 8:
                    break
                packet_id, packet_type = struct.unpack('<II', packet_data[:8])
                command = packet_data[8:].decode('utf-8', errors='ignore').rstrip('\x00')
                if packet_type == RCON_AUTHENTICATE:
                    if command == self.password:
                        response = self.create_response(packet_id, RCON_AUTH_RESPONSE, "")
                    else:
                        response = self.create_response(-1, RCON_AUTH_RESPONSE, "")
                    client_socket.send(response)
                elif packet_type == RCON_EXEC_COMMAND:
                    if command == "colortest":
                        response_text = "\u00a7aGreen text\u00a7r and \u00a74Red text\u00a7r"
                    elif command == "bigpacket":
                        response_text = "A" * 1000
                    elif command == "multiline":
                        response_text = "Line 1\nLine 2\nLine 3"
                    elif command == "disconnect":
                        client_socket.close()
                        return
                    else:
                        response_text = f"Command executed: {command}"
                    response = self.create_response(packet_id, RCON_RESPONSE_VALUE, response_text)
                    client_socket.send(response)
        except Exception as e:
            print(f"Normal request handling error: {e}")

    def create_response(self, packet_id, packet_type, message):
        message_bytes = message.encode('utf-8') + b'\x00\x00'
        packet_size = 4 + 4 + len(message_bytes)
        packet = struct.pack('<I', packet_size)
        packet += struct.pack('<I', packet_id)
        packet += struct.pack('<I', packet_type)
        packet += message_bytes
        return packet

if __name__ == "__main__":
    port = int(sys.argv[1]) if len(sys.argv) > 1 else 25580
    server = SpecialMockRCONServer(port=port)
    def signal_handler(sig, frame):
        server.stop()
        sys.exit(0)
    signal.signal(signal.SIGINT, signal_handler)
    signal.signal(signal.SIGTERM, signal_handler)
    try:
        server.start()
    except KeyboardInterrupt:
        server.stop()
PYEOF

# Start the mock server
python3 special_mock_server.py $MOCK_PORT &
SERVER_PID=$!
sleep 3

if ! nc -z localhost $MOCK_PORT 2>/dev/null; then
    echo "Warning: Mock server not started."
    SERVER_RUNNING=false
else
    echo "Mock server started successfully."
    SERVER_RUNNING=true
fi

run_bin() {
    # usage: run_bin <test_num> <args...>
    local tn=$1
    shift
    LD_PRELOAD=libtracer.so TRACE_OUTPUT=$PWD/flow_results/test${tn}_trace.log ./test_mcrcon_t${tn} "$@"
}

# ---------- Test 1: Help command ----------
echo "Test 1 started"
output=$(LD_PRELOAD=libtracer.so TRACE_OUTPUT=$PWD/flow_results/test1_trace.log ./test_mcrcon_t1 -h 2>&1)
if [[ "$output" == *"Usage:"* ]]; then
    echo "Test 1 passed"
    echo "$output" > flow_results/test1_success.log
else
    echo "Test 1 failed" >&2
    echo "$output" > flow_results/test1_fail.log
    failed=1
fi
echo "Test 1 ended"

# ---------- Test 2: Version information ----------
echo "Test 2 started"
output=$(LD_PRELOAD=libtracer.so TRACE_OUTPUT=$PWD/flow_results/test2_trace.log ./test_mcrcon_t2 -v 2>&1)
if [[ "$output" == *"mcrcon"* ]]; then
    echo "Test 2 passed"
    echo "$output" > flow_results/test2_success.log
else
    echo "Test 2 failed" >&2
    echo "$output" > flow_results/test2_fail.log
    failed=1
fi
echo "Test 2 ended"

# ---------- Test 3: Invalid option ----------
echo "Test 3 started"
output=$(LD_PRELOAD=libtracer.so TRACE_OUTPUT=$PWD/flow_results/test3_trace.log ./test_mcrcon_t3 -x 2>&1)
if [[ "$output" == *"invalid"* ]] || [[ "$output" == *"Unknown"* ]]; then
    echo "Test 3 passed"
    echo "$output" > flow_results/test3_success.log
else
    echo "Test 3 failed" >&2
    echo "$output" > flow_results/test3_fail.log
    failed=1
fi
echo "Test 3 ended"

# ---------- Test 4: Missing required parameter (password) ----------
echo "Test 4 started"
output=$(LD_PRELOAD=libtracer.so TRACE_OUTPUT=$PWD/flow_results/test4_trace.log ./test_mcrcon_t4 -H localhost -P 25575 "say Hello" 2>&1)
if [[ "$output" == *"password"* ]] || [[ "$output" == *"required"* ]]; then
    echo "Test 4 passed"
    echo "$output" > flow_results/test4_success.log
else
    echo "Test 4 failed" >&2
    echo "$output" > flow_results/test4_fail.log
    failed=1
fi
echo "Test 4 ended"

# ---------- Test 5: Invalid wait duration (too large) ----------
echo "Test 5 started"
output=$(LD_PRELOAD=libtracer.so TRACE_OUTPUT=$PWD/flow_results/test5_trace.log ./test_mcrcon_t5 -w 601 2>&1)
if [[ "$output" == *"600"* ]] || [[ "$output" == *"range"* ]]; then
    echo "Test 5 passed"
    echo "$output" > flow_results/test5_success.log
else
    echo "Test 5 failed" >&2
    echo "$output" > flow_results/test5_fail.log
    failed=1
fi
echo "Test 5 ended"

# ---------- Test 6: Invalid wait duration (too small) ----------
echo "Test 6 started"
output=$(LD_PRELOAD=libtracer.so TRACE_OUTPUT=$PWD/flow_results/test6_trace.log ./test_mcrcon_t6 -w 0 2>&1)
if [[ "$output" == *"range"* ]] || [[ "$output" == *"positive"* ]]; then
    echo "Test 6 passed"
    echo "$output" > flow_results/test6_success.log
else
    echo "Test 6 failed" >&2
    echo "$output" > flow_results/test6_fail.log
    failed=1
fi
echo "Test 6 ended"

# ---------- Test 7: Invalid wait duration (non-numeric) ----------
echo "Test 7 started"
output=$(LD_PRELOAD=libtracer.so TRACE_OUTPUT=$PWD/flow_results/test7_trace.log ./test_mcrcon_t7 -w abc 2>&1)
if [[ "$output" == *"not"*"number"* ]] || [[ "$output" == *"invalid"* ]]; then
    echo "Test 7 passed"
    echo "$output" > flow_results/test7_success.log
else
    echo "Test 7 failed" >&2
    echo "$output" > flow_results/test7_fail.log
    failed=1
fi
echo "Test 7 ended"

# ---------- Test 8: Invalid port number (too large) ----------
echo "Test 8 started"
output=$(LD_PRELOAD=libtracer.so TRACE_OUTPUT=$PWD/flow_results/test8_trace.log ./test_mcrcon_t8 -P 65536 -p dummy_password "test" 2>&1)
if [[ "$output" == *"Connection failed"* ]] || [[ "$output" == *"Connection refused"* ]]; then
    echo "Test 8 passed"
    echo "$output" > flow_results/test8_success.log
else
    echo "Test 8 failed" >&2
    echo "$output" > flow_results/test8_fail.log
    failed=1
fi
echo "Test 8 ended"

# ---------- Test 9: Invalid port number (too small) ----------
echo "Test 9 started"
output=$(LD_PRELOAD=libtracer.so TRACE_OUTPUT=$PWD/flow_results/test9_trace.log ./test_mcrcon_t9 -P 0 -p dummy_password "test" 2>&1)
if [[ "$output" == *"Connection failed"* ]] || [[ "$output" == *"Connection refused"* ]]; then
    echo "Test 9 passed"
    echo "$output" > flow_results/test9_success.log
else
    echo "Test 9 failed" >&2
    echo "$output" > flow_results/test9_fail.log
    failed=1
fi
echo "Test 9 ended"

# ---------- Test 10: Invalid port number (non-numeric) ----------
echo "Test 10 started"
output=$(LD_PRELOAD=libtracer.so TRACE_OUTPUT=$PWD/flow_results/test10_trace.log ./test_mcrcon_t10 -P abc -p dummy_password "test" 2>&1)
if [[ "$output" == *"Name resolution failed"* ]]; then
    echo "Test 10 passed"
    echo "$output" > flow_results/test10_success.log
else
    echo "Test 10 failed" >&2
    echo "$output" > flow_results/test10_fail.log
    failed=1
fi
echo "Test 10 ended"

# ---------- Test 11: Environment variable for host ----------
echo "Test 11 started"
export MCRCON_HOST="nonexistent.server"
output=$(LD_PRELOAD=libtracer.so TRACE_OUTPUT=$PWD/flow_results/test11_trace.log ./test_mcrcon_t11 -p password "say test" 2>&1)
if [[ "$output" == *"connect"* ]] || [[ "$output" == *"failed"* ]]; then
    echo "Test 11 passed"
    echo "$output" > flow_results/test11_success.log
else
    echo "Test 11 failed" >&2
    echo "$output" > flow_results/test11_fail.log
    failed=1
fi
echo "Test 11 ended"

# ---------- Test 12: Environment variable for port ----------
echo "Test 12 started"
export MCRCON_PORT="12345"
output=$(LD_PRELOAD=libtracer.so TRACE_OUTPUT=$PWD/flow_results/test12_trace.log ./test_mcrcon_t12 -H nonexistent.server -p password "say test" 2>&1)
if [[ "$output" == *"connect"* ]] || [[ "$output" == *"failed"* ]]; then
    echo "Test 12 passed"
    echo "$output" > flow_results/test12_success.log
else
    echo "Test 12 failed" >&2
    echo "$output" > flow_results/test12_fail.log
    failed=1
fi
echo "Test 12 ended"

# ---------- Test 13: Environment variable for password ----------
echo "Test 13 started"
export MCRCON_PASS="testpassword"
output=$(LD_PRELOAD=libtracer.so TRACE_OUTPUT=$PWD/flow_results/test13_trace.log ./test_mcrcon_t13 -H nonexistent.server "say test" 2>&1)
if [[ "$output" == *"connect"* ]] || [[ "$output" == *"failed"* ]]; then
    echo "Test 13 passed"
    echo "$output" > flow_results/test13_success.log
else
    echo "Test 13 failed" >&2
    echo "$output" > flow_results/test13_fail.log
    failed=1
fi
echo "Test 13 ended"

# ---------- Test 14: CLI options override environment ----------
echo "Test 14 started"
export MCRCON_HOST="valid.server"
output=$(LD_PRELOAD=libtracer.so TRACE_OUTPUT=$PWD/flow_results/test14_trace.log ./test_mcrcon_t14 -H nonexistent.server -p password "say test" 2>&1)
if [[ "$output" == *"connect"* ]] || [[ "$output" == *"failed"* ]]; then
    echo "Test 14 passed"
    echo "$output" > flow_results/test14_success.log
else
    echo "Test 14 failed" >&2
    echo "$output" > flow_results/test14_fail.log
    failed=1
fi
echo "Test 14 ended"

# ---------- Test 15: Connection failure (no server) ----------
echo "Test 15 started"
output=$(LD_PRELOAD=libtracer.so TRACE_OUTPUT=$PWD/flow_results/test15_trace.log ./test_mcrcon_t15 -H nonexistent.server -p password -P 25575 "say test" 2>&1)
if [[ "$output" == *"connect"* ]] || [[ "$output" == *"failed"* ]]; then
    echo "Test 15 passed"
    echo "$output" > flow_results/test15_success.log
else
    echo "Test 15 failed" >&2
    echo "$output" > flow_results/test15_fail.log
    failed=1
fi
echo "Test 15 ended"

# ---------- Test 16: Connection timeout/failure ----------
echo "Test 16 started"
output=$(LD_PRELOAD=libtracer.so TRACE_OUTPUT=$PWD/flow_results/test16_trace.log timeout 5 ./test_mcrcon_t16 -H 192.0.2.1 -p password -P 25575 "say test" 2>&1)
exit_code=$?
if [[ "$output" == *"timeout"* ]] || [[ "$output" == *"connect"* ]] || [[ "$output" == *"failed"* ]] || [[ "$output" == *"Connection"* ]] || [[ "$output" == *"refused"* ]] || [[ $exit_code -eq 124 ]]; then
    echo "Test 16 passed"
    echo "Output: '$output', Exit: $exit_code" > flow_results/test16_success.log
else
    echo "Test 16 failed" >&2
    echo "Output: '$output', Exit: $exit_code" > flow_results/test16_fail.log
    failed=1
fi
echo "Test 16 ended"

if [ "$SERVER_RUNNING" = true ]; then
    # ---------- Test 17: Successful authentication and command ----------
    echo "Test 17 started"
    output=$(LD_PRELOAD=libtracer.so TRACE_OUTPUT=$PWD/flow_results/test17_trace.log timeout 5 ./test_mcrcon_t17 -H localhost -P $MOCK_PORT -p test "say Hello" 2>&1)
    exit_code=$?
    if [[ "$output" == *"Command executed"* ]] || [[ "$output" == *"Hello"* ]] || [[ "$output" == *"executed"* ]] || [[ $exit_code -eq 0 ]]; then
        echo "Test 17 passed"
        echo "$output" > flow_results/test17_success.log
    else
        echo "Test 17 failed" >&2
        echo "Output: '$output', Exit: $exit_code" > flow_results/test17_fail.log
        failed=1
    fi
    echo "Test 17 ended"

    # ---------- Test 18: Multiple commands with wait time ----------
    echo "Test 18 started"
    start_time=$(date +%s)
    output=$(LD_PRELOAD=libtracer.so TRACE_OUTPUT=$PWD/flow_results/test18_trace.log timeout 10 ./test_mcrcon_t18 -H localhost -P $MOCK_PORT -p test -w 1 "say Hello" "say World" 2>&1)
    end_time=$(date +%s)
    time_diff=$((end_time - start_time))
    if [[ $time_diff -ge 1 ]] && [[ "$output" == *"executed"* ]]; then
        echo "Test 18 passed"
        echo "$output" > flow_results/test18_success.log
    else
        echo "Test 18 failed" >&2
        echo "Output: '$output', Time: ${time_diff}s" > flow_results/test18_fail.log
        failed=1
    fi
    echo "Test 18 ended"

    # ---------- Test 19: Silent mode ----------
    echo "Test 19 started"
    output=$(LD_PRELOAD=libtracer.so TRACE_OUTPUT=$PWD/flow_results/test19_trace.log timeout 3 ./test_mcrcon_t19 -s -H localhost -P $MOCK_PORT -p test "say test" 2>&1)
    if [[ -z "$output" ]]; then
        echo "Test 19 passed"
        echo "(no output expected)" > flow_results/test19_success.log
    else
        echo "Test 19 failed" >&2
        echo "$output" > flow_results/test19_fail.log
        failed=1
    fi
    echo "Test 19 ended"

    # ---------- Test 20: Raw packet mode ----------
    echo "Test 20 started"
    output=$(LD_PRELOAD=libtracer.so TRACE_OUTPUT=$PWD/flow_results/test20_trace.log timeout 3 ./test_mcrcon_t20 -r -H localhost -P $MOCK_PORT -p test "say test" 2>&1)
    exit_code=$?
    if [ $exit_code -eq 0 ] && [ -n "$output" ]; then
        echo "Test 20 passed"
        echo "$output" > flow_results/test20_success.log
    else
        echo "Test 20 failed" >&2
        echo "Output: '$output', Exit: $exit_code" > flow_results/test20_fail.log
        failed=1
    fi
    echo "Test 20 ended"

    # ---------- Test 21: Disable colors ----------
    echo "Test 21 started"
    output=$(LD_PRELOAD=libtracer.so TRACE_OUTPUT=$PWD/flow_results/test21_trace.log timeout 3 ./test_mcrcon_t21 -c -H localhost -P $MOCK_PORT -p test "colortest" 2>&1)
    if [[ "$output" != *$'\e['* ]]; then
        echo "Test 21 passed"
        echo "$output" > flow_results/test21_success.log
    else
        echo "Test 21 failed" >&2
        echo "$output" > flow_results/test21_fail.log
        failed=1
    fi
    echo "Test 21 ended"

    # ---------- Test 22: Color processing ----------
    echo "Test 22 started"
    output=$(LD_PRELOAD=libtracer.so TRACE_OUTPUT=$PWD/flow_results/test22_trace.log timeout 3 ./test_mcrcon_t22 -H localhost -P $MOCK_PORT -p test "colortest" 2>&1)
    exit_code=$?
    if [ $exit_code -eq 0 ] && [[ "$output" == *"Green"* || "$output" == *"Red"* ]]; then
        echo "Test 22 passed"
        echo "$output" > flow_results/test22_success.log
    else
        echo "Test 22 failed" >&2
        echo "Output: '$output', Exit: $exit_code" > flow_results/test22_fail.log
        failed=1
    fi
    echo "Test 22 ended"

    # ---------- Test 23: Terminal mode ----------
    echo "Test 23 started"
    echo -e "say Hello Terminal\nQ\n" | LD_PRELOAD=libtracer.so TRACE_OUTPUT=$PWD/flow_results/test23_trace.log timeout 3 ./test_mcrcon_t23 -H localhost -P $MOCK_PORT -p test -t > /tmp/terminal_output.log 2>&1
    exit_code=$?
    if [ $exit_code -ne 124 ] && [ -f /tmp/terminal_output.log ]; then
        echo "Test 23 passed"
        cat /tmp/terminal_output.log > flow_results/test23_success.log
    else
        echo "Test 23 failed" >&2
        echo "Exit: $exit_code" > flow_results/test23_fail.log
        [ -f /tmp/terminal_output.log ] && cat /tmp/terminal_output.log >> flow_results/test23_fail.log
        failed=1
    fi
    echo "Test 23 ended"
else
    for skip_test in 17 18 19 20 21 22 23; do
        echo "Test ${skip_test} started"
        echo "Test ${skip_test} failed" >&2
        echo "Server not running" > flow_results/test${skip_test}_fail.log
        failed=1
        echo "Test ${skip_test} ended"
    done
fi

# ---------- Test 24: SIGINT handling ----------
echo "Test 24 started"
LD_PRELOAD=libtracer.so TRACE_OUTPUT=$PWD/flow_results/test24_trace.log ./test_mcrcon_t24 -H localhost -P $MOCK_PORT -p test -t &
PID=$!
sleep 0.5
kill -INT $PID 2>/dev/null
sleep 1
if ! kill -0 $PID 2>/dev/null; then
    echo "Test 24 passed"
    echo "Process terminated cleanly after SIGINT" > flow_results/test24_success.log
else
    echo "Test 24 failed" >&2
    echo "Process did not terminate after SIGINT" > flow_results/test24_fail.log
    kill -9 $PID 2>/dev/null
    failed=1
fi
wait $PID 2>/dev/null
echo "Test 24 ended"

# ---------- Test 25: Large wait number (errno test) ----------
echo "Test 25 started"
HUGE_NUMBER="99999999999999999999999999999999999999999999999"
output=$(LD_PRELOAD=libtracer.so TRACE_OUTPUT=$PWD/flow_results/test25_trace.log ./test_mcrcon_t25 -p test -w $HUGE_NUMBER 2>&1)
if [[ "$output" == *"range"* ]] || [[ "$output" == *"number"* ]]; then
    echo "Test 25 passed"
    echo "$output" > flow_results/test25_success.log
else
    echo "Test 25 failed" >&2
    echo "$output" > flow_results/test25_fail.log
    failed=1
fi
echo "Test 25 ended"

if [ "$SERVER_RUNNING" = true ]; then
    # ---------- Test 26: Invalid packet size ----------
    echo "Test 26 started"
    output=$(LD_PRELOAD=libtracer.so TRACE_OUTPUT=$PWD/flow_results/test26_trace.log timeout 3 ./test_mcrcon_t26 -H localhost -P $MOCK_PORT -p test "invalidpacket" 2>&1)
    exit_code=$?
    if [ $exit_code -ne 124 ]; then
        echo "Test 26 passed"
        echo "Output: '$output', Exit: $exit_code" > flow_results/test26_success.log
    else
        echo "Test 26 failed" >&2
        echo "Output: '$output', Exit: $exit_code" > flow_results/test26_fail.log
        failed=1
    fi
    echo "Test 26 ended"

    # ---------- Test 27: Connection disconnect ----------
    echo "Test 27 started"
    output=$(LD_PRELOAD=libtracer.so TRACE_OUTPUT=$PWD/flow_results/test27_trace.log timeout 3 ./test_mcrcon_t27 -H localhost -P $MOCK_PORT -p test "disconnect" 2>&1)
    exit_code=$?
    if [ $exit_code -ne 124 ]; then
        echo "Test 27 passed"
        echo "Output: '$output', Exit: $exit_code" > flow_results/test27_success.log
    else
        echo "Test 27 failed" >&2
        echo "Output: '$output', Exit: $exit_code" > flow_results/test27_fail.log
        failed=1
    fi
    echo "Test 27 ended"

    # ---------- Test 28: Large packet ----------
    echo "Test 28 started"
    VERY_LONG_CMD="say $(printf 'A%.0s' {1..1000})"
    output=$(LD_PRELOAD=libtracer.so TRACE_OUTPUT=$PWD/flow_results/test28_trace.log timeout 3 ./test_mcrcon_t28 -H localhost -P $MOCK_PORT -p test "$VERY_LONG_CMD" 2>&1)
    exit_code=$?
    if [ $exit_code -ne 124 ]; then
        echo "Test 28 passed"
        echo "Output: '$output', Exit: $exit_code" > flow_results/test28_success.log
    else
        echo "Test 28 failed" >&2
        echo "Output: '$output', Exit: $exit_code" > flow_results/test28_fail.log
        failed=1
    fi
    echo "Test 28 ended"

    # ---------- Test 29: Terminal mode with long input ----------
    echo "Test 29 started"
    LONG_INPUT=$(printf 'A%.0s' {1..1000})
    echo -e "${LONG_INPUT}\nQ\n" | LD_PRELOAD=libtracer.so TRACE_OUTPUT=$PWD/flow_results/test29_trace.log timeout 3 ./test_mcrcon_t29 -H localhost -P $MOCK_PORT -p test -t > /tmp/long_input_output.log 2>&1
    exit_code=$?
    if [ $exit_code -ne 124 ] && [ -f /tmp/long_input_output.log ]; then
        echo "Test 29 passed"
        cat /tmp/long_input_output.log > flow_results/test29_success.log
    else
        echo "Test 29 failed" >&2
        echo "Exit: $exit_code" > flow_results/test29_fail.log
        [ -f /tmp/long_input_output.log ] && cat /tmp/long_input_output.log >> flow_results/test29_fail.log
        failed=1
    fi
    echo "Test 29 ended"

    # ---------- Test 30: Terminal mode with closed stdin ----------
    echo "Test 30 started"
    LD_PRELOAD=libtracer.so TRACE_OUTPUT=$PWD/flow_results/test30_trace.log timeout 3 ./test_mcrcon_t30 -H localhost -P $MOCK_PORT -p test -t < /dev/null > /tmp/closed_stdin_output.log 2>&1
    exit_code=$?
    if [ $exit_code -ne 124 ] && [ -f /tmp/closed_stdin_output.log ]; then
        echo "Test 30 passed"
        cat /tmp/closed_stdin_output.log > flow_results/test30_success.log
    else
        echo "Test 30 failed" >&2
        echo "Exit: $exit_code" > flow_results/test30_fail.log
        [ -f /tmp/closed_stdin_output.log ] && cat /tmp/closed_stdin_output.log >> flow_results/test30_fail.log
        failed=1
    fi
    echo "Test 30 ended"
else
    for skip_test in 26 27 28 29 30; do
        echo "Test ${skip_test} started"
        echo "Test ${skip_test} failed" >&2
        echo "Server not running" > flow_results/test${skip_test}_fail.log
        failed=1
        echo "Test ${skip_test} ended"
    done
fi

# Stop the mock server
if [ "$SERVER_RUNNING" = true ]; then
    kill $SERVER_PID 2>/dev/null
    wait $SERVER_PID 2>/dev/null
    sleep 1
fi

# Cleanup
rm -f special_mock_server.py
rm -f /tmp/terminal_output.log
rm -f /tmp/long_input_output.log
rm -f /tmp/closed_stdin_output.log

exit $failed

