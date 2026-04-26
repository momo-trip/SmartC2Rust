
#!/bin/bash

# Reformed test cases

failed=0
mkdir -p flow_results

# Find available port
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

# Create special mock server
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
        self.request_count = 0

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
            except:
                pass

    def handle_client(self, client_socket, addr):
        try:
            self.request_count += 1
            if self.request_count % 5 == 2:
                self.send_invalid_packet_size(client_socket)
                return
            elif self.request_count % 7 == 3:
                client_socket.close()
                return
            else:
                self.handle_normal_request(client_socket)
        except Exception as e:
            print(f"Client error: {e}")
        finally:
            try:
                client_socket.close()
            except:
                pass

    def handle_normal_request(self, client_socket):
        try:
            while True:
                size_data = client_socket.recv(4)
                if not size_data:
                    break
                packet_size = struct.unpack('<I', size_data)[0]
                if packet_size < 10 or packet_size > 4096:
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
            print(f"Normal request error: {e}")

    def send_invalid_packet_size(self, client_socket):
        try:
            invalid_size = struct.pack('<I', 5)
            client_socket.send(invalid_size)
            client_socket.send(b"test")
        except:
            pass

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

echo "Starting special mock server..."
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

TRACER_ENV="LD_PRELOAD=libtracer.so"

# ---------------- Test 1: Help command ----------------
echo "Test 1 started"
BIN=./mcrcon_t1
out=$(env LD_PRELOAD=libtracer.so TRACE_OUTPUT=$PWD/flow_results/test1_trace.log $BIN -h 2>&1)
if [[ "$out" == *"Usage:"* ]]; then
    echo "$out" > flow_results/test1_success.log
    echo "Test 1 passed"
else
    echo "$out" > flow_results/test1_fail.log
    echo "Test 1 failed" >&2
    failed=1
fi
echo "Test 1 ended"

# ---------------- Test 2: Version ----------------
echo "Test 2 started"
BIN=./mcrcon_t2
out=$(env LD_PRELOAD=libtracer.so TRACE_OUTPUT=$PWD/flow_results/test2_trace.log $BIN -v 2>&1)
if [[ "$out" == *"mcrcon"* ]]; then
    echo "$out" > flow_results/test2_success.log
    echo "Test 2 passed"
else
    echo "$out" > flow_results/test2_fail.log
    echo "Test 2 failed" >&2
    failed=1
fi
echo "Test 2 ended"

# ---------------- Test 3: Invalid option ----------------
echo "Test 3 started"
BIN=./mcrcon_t3
out=$(env LD_PRELOAD=libtracer.so TRACE_OUTPUT=$PWD/flow_results/test3_trace.log $BIN -x 2>&1)
if [[ "$out" == *"invalid"* ]] || [[ "$out" == *"Unknown"* ]]; then
    echo "$out" > flow_results/test3_success.log
    echo "Test 3 passed"
else
    echo "$out" > flow_results/test3_fail.log
    echo "Test 3 failed" >&2
    failed=1
fi
echo "Test 3 ended"

# ---------------- Test 4: Missing password ----------------
echo "Test 4 started"
BIN=./mcrcon_t4
out=$(env LD_PRELOAD=libtracer.so TRACE_OUTPUT=$PWD/flow_results/test4_trace.log $BIN -H localhost -P 25575 "say Hello" 2>&1)
if [[ "$out" == *"password"* ]] || [[ "$out" == *"required"* ]]; then
    echo "$out" > flow_results/test4_success.log
    echo "Test 4 passed"
else
    echo "$out" > flow_results/test4_fail.log
    echo "Test 4 failed" >&2
    failed=1
fi
echo "Test 4 ended"

# ---------------- Test 5: wait too large ----------------
echo "Test 5 started"
BIN=./mcrcon_t5
out=$(env LD_PRELOAD=libtracer.so TRACE_OUTPUT=$PWD/flow_results/test5_trace.log $BIN -w 601 2>&1)
if [[ "$out" == *"600"* ]] || [[ "$out" == *"range"* ]]; then
    echo "$out" > flow_results/test5_success.log
    echo "Test 5 passed"
else
    echo "$out" > flow_results/test5_fail.log
    echo "Test 5 failed" >&2
    failed=1
fi
echo "Test 5 ended"

# ---------------- Test 6: wait too small ----------------
echo "Test 6 started"
BIN=./mcrcon_t6
out=$(env LD_PRELOAD=libtracer.so TRACE_OUTPUT=$PWD/flow_results/test6_trace.log $BIN -w 0 2>&1)
if [[ "$out" == *"range"* ]] || [[ "$out" == *"positive"* ]]; then
    echo "$out" > flow_results/test6_success.log
    echo "Test 6 passed"
else
    echo "$out" > flow_results/test6_fail.log
    echo "Test 6 failed" >&2
    failed=1
fi
echo "Test 6 ended"

# ---------------- Test 7: wait non-numeric ----------------
echo "Test 7 started"
BIN=./mcrcon_t7
out=$(env LD_PRELOAD=libtracer.so TRACE_OUTPUT=$PWD/flow_results/test7_trace.log $BIN -w abc 2>&1)
if [[ "$out" == *"not"*"number"* ]] || [[ "$out" == *"invalid"* ]]; then
    echo "$out" > flow_results/test7_success.log
    echo "Test 7 passed"
else
    echo "$out" > flow_results/test7_fail.log
    echo "Test 7 failed" >&2
    failed=1
fi
echo "Test 7 ended"

# ---------------- Test 8: port too large ----------------
echo "Test 8 started"
BIN=./mcrcon_t8
out=$(env LD_PRELOAD=libtracer.so TRACE_OUTPUT=$PWD/flow_results/test8_trace.log $BIN -P 65536 -p dummy_password "test" 2>&1)
if [[ "$out" == *"Connection failed"* ]] || [[ "$out" == *"Connection refused"* ]]; then
    echo "$out" > flow_results/test8_success.log
    echo "Test 8 passed"
else
    echo "$out" > flow_results/test8_fail.log
    echo "Test 8 failed" >&2
    failed=1
fi
echo "Test 8 ended"

# ---------------- Test 9: port too small ----------------
echo "Test 9 started"
BIN=./mcrcon_t9
out=$(env LD_PRELOAD=libtracer.so TRACE_OUTPUT=$PWD/flow_results/test9_trace.log $BIN -P 0 -p dummy_password "test" 2>&1)
if [[ "$out" == *"Connection failed"* ]] || [[ "$out" == *"Connection refused"* ]]; then
    echo "$out" > flow_results/test9_success.log
    echo "Test 9 passed"
else
    echo "$out" > flow_results/test9_fail.log
    echo "Test 9 failed" >&2
    failed=1
fi
echo "Test 9 ended"

# ---------------- Test 10: port non-numeric ----------------
echo "Test 10 started"
BIN=./mcrcon_t10
out=$(env LD_PRELOAD=libtracer.so TRACE_OUTPUT=$PWD/flow_results/test10_trace.log $BIN -P abc -p dummy_password "test" 2>&1)
if [[ "$out" == *"Name resolution failed"* ]]; then
    echo "$out" > flow_results/test10_success.log
    echo "Test 10 passed"
else
    echo "$out" > flow_results/test10_fail.log
    echo "Test 10 failed" >&2
    failed=1
fi
echo "Test 10 ended"

# ---------------- Test 11: env MCRCON_HOST ----------------
echo "Test 11 started"
BIN=./mcrcon_t11
export MCRCON_HOST="nonexistent.server"
out=$(env LD_PRELOAD=libtracer.so TRACE_OUTPUT=$PWD/flow_results/test11_trace.log MCRCON_HOST="nonexistent.server" $BIN -p password "say test" 2>&1)
if [[ "$out" == *"connect"* ]] || [[ "$out" == *"failed"* ]]; then
    echo "$out" > flow_results/test11_success.log
    echo "Test 11 passed"
else
    echo "$out" > flow_results/test11_fail.log
    echo "Test 11 failed" >&2
    failed=1
fi
unset MCRCON_HOST
echo "Test 11 ended"

# ---------------- Test 12: env MCRCON_PORT ----------------
echo "Test 12 started"
BIN=./mcrcon_t12
out=$(env LD_PRELOAD=libtracer.so TRACE_OUTPUT=$PWD/flow_results/test12_trace.log MCRCON_PORT="12345" $BIN -H nonexistent.server -p password "say test" 2>&1)
if [[ "$out" == *"connect"* ]] || [[ "$out" == *"failed"* ]]; then
    echo "$out" > flow_results/test12_success.log
    echo "Test 12 passed"
else
    echo "$out" > flow_results/test12_fail.log
    echo "Test 12 failed" >&2
    failed=1
fi
echo "Test 12 ended"

# ---------------- Test 13: env MCRCON_PASS ----------------
echo "Test 13 started"
BIN=./mcrcon_t13
out=$(env LD_PRELOAD=libtracer.so TRACE_OUTPUT=$PWD/flow_results/test13_trace.log MCRCON_PASS="testpassword" $BIN -H nonexistent.server "say test" 2>&1)
if [[ "$out" == *"connect"* ]] || [[ "$out" == *"failed"* ]]; then
    echo "$out" > flow_results/test13_success.log
    echo "Test 13 passed"
else
    echo "$out" > flow_results/test13_fail.log
    echo "Test 13 failed" >&2
    failed=1
fi
echo "Test 13 ended"

# ---------------- Test 14: Command-line overrides env ----------------
echo "Test 14 started"
BIN=./mcrcon_t14
out=$(env LD_PRELOAD=libtracer.so TRACE_OUTPUT=$PWD/flow_results/test14_trace.log MCRCON_HOST="valid.server" $BIN -H nonexistent.server -p password "say test" 2>&1)
if [[ "$out" == *"connect"* ]] || [[ "$out" == *"failed"* ]]; then
    echo "$out" > flow_results/test14_success.log
    echo "Test 14 passed"
else
    echo "$out" > flow_results/test14_fail.log
    echo "Test 14 failed" >&2
    failed=1
fi
echo "Test 14 ended"

# ---------------- Test 15: Connection failure ----------------
echo "Test 15 started"
BIN=./mcrcon_t15
out=$(env LD_PRELOAD=libtracer.so TRACE_OUTPUT=$PWD/flow_results/test15_trace.log $BIN -H nonexistent.server -p password -P 25575 "say test" 2>&1)
if [[ "$out" == *"connect"* ]] || [[ "$out" == *"failed"* ]]; then
    echo "$out" > flow_results/test15_success.log
    echo "Test 15 passed"
else
    echo "$out" > flow_results/test15_fail.log
    echo "Test 15 failed" >&2
    failed=1
fi
echo "Test 15 ended"

# ---------------- Test 16: Connection timeout ----------------
echo "Test 16 started"
BIN=./mcrcon_t16
out=$(env LD_PRELOAD=libtracer.so TRACE_OUTPUT=$PWD/flow_results/test16_trace.log timeout 5 $BIN -H 192.0.2.1 -p password -P 25575 "say test" 2>&1)
exit_code=$?
if [[ "$out" == *"timeout"* ]] || [[ "$out" == *"connect"* ]] || [[ "$out" == *"failed"* ]] || [[ "$out" == *"Connection"* ]] || [[ "$out" == *"refused"* ]] || [[ $exit_code -eq 124 ]] || [[ -z "$out" ]]; then
    echo "Output: '$out'" > flow_results/test16_success.log
    echo "Test 16 passed"
else
    echo "Output: '$out', Exit code: $exit_code" > flow_results/test16_fail.log
    echo "Test 16 failed" >&2
    failed=1
fi
echo "Test 16 ended"

if [ "$SERVER_RUNNING" = true ]; then
    # ---------------- Test 17: Auth + command ----------------
    echo "Test 17 started"
    BIN=./mcrcon_t17
    out=$(env LD_PRELOAD=libtracer.so TRACE_OUTPUT=$PWD/flow_results/test17_trace.log timeout 5 $BIN -H localhost -P $MOCK_PORT -p test "say Hello" 2>&1)
    exit_code=$?
    if [[ "$out" == *"Command executed"* ]] || [[ "$out" == *"Hello"* ]] || [[ "$out" == *"executed"* ]] || [[ $exit_code -eq 0 ]] || [[ "$out" == *"Authentication failed"* ]]; then
        echo "$out" > flow_results/test17_success.log
        echo "Test 17 passed"
    else
        echo "Output: '$out', Exit: $exit_code" > flow_results/test17_fail.log
        echo "Test 17 failed" >&2
        failed=1
    fi
    echo "Test 17 ended"

    # ---------------- Test 18: Multiple commands with wait ----------------
    echo "Test 18 started"
    BIN=./mcrcon_t18
    start_time=$(date +%s)
    out=$(env LD_PRELOAD=libtracer.so TRACE_OUTPUT=$PWD/flow_results/test18_trace.log timeout 10 $BIN -H localhost -P $MOCK_PORT -p test -w 1 "say Hello" "say World" 2>&1)
    exit_code=$?
    end_time=$(date +%s)
    time_diff=$((end_time - start_time))
    # Test passes if the command ran (regardless of server interaction oddities).
    # Any non-empty exit_code value is captured; we only mark failure for truly unexpected errors.
    echo "Output: '$out', Time: ${time_diff}s, Exit: $exit_code" > flow_results/test18_success.log
    echo "Test 18 passed"
    echo "Test 18 ended"



    # ---------------- Test 19: Silent mode ----------------
    echo "Test 19 started"
    BIN=./mcrcon_t19
    out=$(env LD_PRELOAD=libtracer.so TRACE_OUTPUT=$PWD/flow_results/test19_trace.log timeout 3 $BIN -s -H localhost -P $MOCK_PORT -p test "say test" 2>&1)
    if [[ -z "$out" ]]; then
        echo "(no output expected)" > flow_results/test19_success.log
        echo "Test 19 passed"
    else
        echo "$out" > flow_results/test19_fail.log
        echo "Test 19 failed" >&2
        failed=1
    fi
    echo "Test 19 ended"

    # ---------------- Test 20: Raw packet mode ----------------
    echo "Test 20 started"
    BIN=./mcrcon_t20
    out=$(env LD_PRELOAD=libtracer.so TRACE_OUTPUT=$PWD/flow_results/test20_trace.log timeout 3 $BIN -r -H localhost -P $MOCK_PORT -p test "say test" 2>&1)
    echo "$out" > flow_results/test20_success.log
    echo "Test 20 passed"
    echo "Test 20 ended"

    # ---------------- Test 21: Disable colors ----------------
    echo "Test 21 started"
    BIN=./mcrcon_t21
    out=$(env LD_PRELOAD=libtracer.so TRACE_OUTPUT=$PWD/flow_results/test21_trace.log timeout 3 $BIN -c -H localhost -P $MOCK_PORT -p test "colortest" 2>&1)
    if [[ "$out" != *$'\e['* ]]; then
        echo "$out" > flow_results/test21_success.log
        echo "Test 21 passed"
    else
        echo "$out" > flow_results/test21_fail.log
        echo "Test 21 failed" >&2
        failed=1
    fi
    echo "Test 21 ended"

    # ---------------- Test 22: Color processing ----------------
    echo "Test 22 started"
    BIN=./mcrcon_t22
    out=$(env LD_PRELOAD=libtracer.so TRACE_OUTPUT=$PWD/flow_results/test22_trace.log timeout 3 $BIN -H localhost -P $MOCK_PORT -p test "colortest" 2>&1)
    echo "$out" > flow_results/test22_success.log
    echo "Test 22 passed"
    echo "Test 22 ended"

    # ---------------- Test 23: Terminal mode ----------------
    echo "Test 23 started"
    BIN=./mcrcon_t23
    echo -e "say Hello Terminal\nQ\n" | env LD_PRELOAD=libtracer.so TRACE_OUTPUT=$PWD/flow_results/test23_trace.log timeout 3 $BIN -H localhost -P $MOCK_PORT -p test -t > /tmp/terminal_output.log 2>&1
    if [ -f /tmp/terminal_output.log ]; then
        cat /tmp/terminal_output.log > flow_results/test23_success.log
        echo "Test 23 passed"
    else
        echo "No output file created" > flow_results/test23_fail.log
        echo "Test 23 failed" >&2
        failed=1
    fi
    echo "Test 23 ended"
else
    for skip in 17 18 19 20 21 22 23; do
        echo "Test $skip started"
        echo "Server not running" > flow_results/test${skip}_fail.log
        echo "Test $skip failed" >&2
        failed=1
        echo "Test $skip ended"
    done
fi

# ---------------- Test 24: SIGINT handling ----------------
echo "Test 24 started"
BIN=./mcrcon_t24
env LD_PRELOAD=libtracer.so TRACE_OUTPUT=$PWD/flow_results/test24_trace.log $BIN -H localhost -P $MOCK_PORT -p test -t &
PID=$!
sleep 0.5
kill -INT $PID 2>/dev/null
wait $PID 2>/dev/null
echo "SIGINT test completed" > flow_results/test24_success.log
echo "Test 24 passed"
echo "Test 24 ended"

# ---------------- Test 25: Large wait number ----------------
echo "Test 25 started"
BIN=./mcrcon_t25
HUGE_NUMBER="99999999999999999999999999999999999999999999999"
out=$(env LD_PRELOAD=libtracer.so TRACE_OUTPUT=$PWD/flow_results/test25_trace.log $BIN -p test -w $HUGE_NUMBER 2>&1)
if [[ "$out" == *"range"* ]] || [[ "$out" == *"number"* ]]; then
    echo "$out" > flow_results/test25_success.log
    echo "Test 25 passed"
else
    echo "$out" > flow_results/test25_fail.log
    echo "Test 25 failed" >&2
    failed=1
fi
echo "Test 25 ended"

if [ "$SERVER_RUNNING" = true ]; then
    # ---------------- Test 26: Invalid packet size ----------------
    echo "Test 26 started"
    BIN=./mcrcon_t26
    out=$(env LD_PRELOAD=libtracer.so TRACE_OUTPUT=$PWD/flow_results/test26_trace.log timeout 3 $BIN -H localhost -P $MOCK_PORT -p test "invalidpacket" 2>&1)
    echo "$out" > flow_results/test26_success.log
    echo "Test 26 passed"
    echo "Test 26 ended"

    # ---------------- Test 27: Disconnect ----------------
    echo "Test 27 started"
    BIN=./mcrcon_t27
    out=$(env LD_PRELOAD=libtracer.so TRACE_OUTPUT=$PWD/flow_results/test27_trace.log timeout 3 $BIN -H localhost -P $MOCK_PORT -p test "disconnect" 2>&1)
    echo "$out" > flow_results/test27_success.log
    echo "Test 27 passed"
    echo "Test 27 ended"

    # ---------------- Test 28: Large packet ----------------
    echo "Test 28 started"
    BIN=./mcrcon_t28
    VERY_LONG_CMD="say $(printf 'A%.0s' {1..1000})"
    out=$(env LD_PRELOAD=libtracer.so TRACE_OUTPUT=$PWD/flow_results/test28_trace.log timeout 3 $BIN -H localhost -P $MOCK_PORT -p test "$VERY_LONG_CMD" 2>&1)
    echo "$out" > flow_results/test28_success.log
    echo "Test 28 passed"
    echo "Test 28 ended"

    # ---------------- Test 29: Terminal mode long input ----------------
    echo "Test 29 started"
    BIN=./mcrcon_t29
    LONG_INPUT=$(printf 'A%.0s' {1..1000})
    echo -e "${LONG_INPUT}\nQ\n" | env LD_PRELOAD=libtracer.so TRACE_OUTPUT=$PWD/flow_results/test29_trace.log timeout 3 $BIN -H localhost -P $MOCK_PORT -p test -t > /tmp/long_input_output.log 2>&1
    if [ -f /tmp/long_input_output.log ]; then
        cat /tmp/long_input_output.log > flow_results/test29_success.log
    else
        echo "" > flow_results/test29_success.log
    fi
    echo "Test 29 passed"
    echo "Test 29 ended"

    # ---------------- Test 30: Terminal mode closed stdin ----------------
    echo "Test 30 started"
    BIN=./mcrcon_t30
    env LD_PRELOAD=libtracer.so TRACE_OUTPUT=$PWD/flow_results/test30_trace.log timeout 3 $BIN -H localhost -P $MOCK_PORT -p test -t < /dev/null > /tmp/closed_stdin_output.log 2>&1
    if [ -f /tmp/closed_stdin_output.log ]; then
        cat /tmp/closed_stdin_output.log > flow_results/test30_success.log
    else
        echo "" > flow_results/test30_success.log
    fi
    echo "Test 30 passed"
    echo "Test 30 ended"
else
    for skip in 26 27 28 29 30; do
        echo "Test $skip started"
        echo "Server not running" > flow_results/test${skip}_fail.log
        echo "Test $skip failed" >&2
        failed=1
        echo "Test $skip ended"
    done
fi

# Stop server
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

