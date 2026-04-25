
#!/bin/bash

# Reformed test cases

failed=0
mkdir -p flow_results

# Set up xsel mock to capture clipboard output
cat > xsel << 'EOF'
#!/bin/bash
cat > /tmp/yank_clipboard
echo "Content saved to clipboard (mock)" >&2
EOF
chmod +x xsel
export PATH="$(pwd):$PATH"

if ! command -v expect &> /dev/null; then
    echo "ERROR: expect command not found. Install with: sudo apt-get install expect" >&2
    exit 1
fi

# Helper: run a test case
# Args: test_num, description, expected, expect_script, binary_name
run_case() {
    local test_num="$1"
    local description="$2"
    local expected="$3"
    local expect_script="$4"
    local binary_name="$5"

    echo "Test ${test_num} started"
    local log_tmp="flow_results/test${test_num}_tmp.log"
    : > "$log_tmp"
    echo "Description: $description" >> "$log_tmp"
    echo "Expected: '$expected'" >> "$log_tmp"

    rm -f /tmp/yank_clipboard

    LD_PRELOAD=libtracer.so TRACE_OUTPUT="$PWD/flow_results/test${test_num}_trace.log" \
        bash -c "$expect_script" >> "$log_tmp" 2>&1

    local actual
    if [ "$expected" = "EXIT_WITHOUT_CLIPBOARD" ]; then
        if [ ! -f /tmp/yank_clipboard ]; then
            actual="EXIT_WITHOUT_CLIPBOARD"
        else
            actual="$(cat /tmp/yank_clipboard 2>/dev/null)"
        fi
    else
        actual="$(cat /tmp/yank_clipboard 2>/dev/null)"
    fi
    echo "Actual: '$actual'" >> "$log_tmp"

    if [ "$actual" = "$expected" ]; then
        echo "Test ${test_num} passed"
        mv "$log_tmp" "flow_results/test${test_num}_success.log"
    else
        echo "Test ${test_num} failed" >&2
        echo "Test ${test_num} failed"
        failed=1
        mv "$log_tmp" "flow_results/test${test_num}_fail.log"
    fi
    rm -f /tmp/yank_clipboard
    echo "Test ${test_num} ended"
}

# Test 01: Homebrew test case (key=value, select value)
run_case "01" "Homebrew test case" "value" 'expect << EOF
spawn bash -c "echo \"key=value\" | ./test_yank_t1 -d \"=\""
set timeout 5
expect -re ".*" {
    send "\016"
    sleep 0.2
    send "\r"
    expect eof
}
EOF' "test_yank_t1"

# Test 02: Basic selection (first word)
run_case "02" "Basic selection" "hello" 'expect << '\''EOF'\''
spawn bash -c "echo '\''hello world test data'\'' | ./test_yank_t2"
set timeout 5
expect -re ".*" { send "\r"; expect eof }
EOF' "test_yank_t2"

# Test 03: Multi-line selection
run_case "03" "Multi-line selection" "line1" 'expect << '\''EOF'\''
spawn bash -c "echo -e '\''line1\\nline2\\nline3'\'' | ./test_yank_t3"
set timeout 5
expect -re ".*" { send "\r"; expect eof }
EOF' "test_yank_t3"

# Test 04: Word selection
run_case "04" "Word selection" "word1" 'expect << '\''EOF'\''
spawn bash -c "echo '\''word1 word2 word3'\'' | ./test_yank_t4"
set timeout 5
expect -re ".*" { send "\r"; expect eof }
EOF' "test_yank_t4"

# Test 05: Right navigation
run_case "05" "Right navigation (l key)" "word2" 'expect << '\''EOF'\''
spawn bash -c "echo '\''word1 word2 word3'\'' | ./test_yank_t5"
set timeout 5
expect -re ".*" {
    send "l"
    sleep 0.2
    send "\r"
    expect eof
}
EOF' "test_yank_t5"

# Test 06: Left navigation
run_case "06" "Left navigation (h key)" "word1" 'expect << '\''EOF'\''
spawn bash -c "echo '\''word1 word2 word3'\'' | ./test_yank_t6"
set timeout 5
expect -re ".*" {
    send "l"
    sleep 0.2
    send "h"
    sleep 0.2
    send "\r"
    expect eof
}
EOF' "test_yank_t6"

# Test 07: Go to end (G)
run_case "07" "Go to end (G key)" "fourth" 'expect << '\''EOF'\''
spawn bash -c "echo '\''first second third fourth'\'' | ./test_yank_t7"
set timeout 5
expect -re ".*" {
    send "G"
    sleep 0.2
    send "\r"
    expect eof
}
EOF' "test_yank_t7"

# Test 08: Go to beginning (g)
run_case "08" "Go to beginning (g key)" "first" 'expect << '\''EOF'\''
spawn bash -c "echo '\''first second third fourth'\'' | ./test_yank_t8"
set timeout 5
expect -re ".*" {
    send "G"
    sleep 0.2
    send "g"
    sleep 0.2
    send "\r"
    expect eof
}
EOF' "test_yank_t8"

# Test 09: Down navigation
run_case "09" "Down navigation (j key)" "line2" 'expect << '\''EOF'\''
spawn bash -c "printf '\''line1 word1 word2\nline2 word3 word4\nline3 word5 word6'\'' | ./test_yank_t9"
set timeout 5
expect -re ".*" {
    send "j"
    sleep 0.2
    send "\r"
    expect eof
}
EOF' "test_yank_t9"

# Test 10: Up navigation
run_case "10" "Up navigation (k key)" "line1" 'expect << '\''EOF'\''
spawn bash -c "printf '\''line1 word1 word2\nline2 word3 word4\nline3 word5 word6'\'' | ./test_yank_t10"
set timeout 5
expect -re ".*" {
    send "j"
    sleep 0.2
    send "k"
    sleep 0.2
    send "\r"
    expect eof
}
EOF' "test_yank_t10"

# Test 11: Arrow key navigation
run_case "11" "Arrow key navigation (right)" "word2" 'expect << '\''EOF'\''
spawn bash -c "echo '\''word1 word2 word3'\'' | ./test_yank_t11"
set timeout 5
expect -re ".*" {
    send "\033\[C"
    sleep 0.2
    send "\r"
    expect eof
}
EOF' "test_yank_t11"

# Test 12: Home with Ctrl-A
run_case "12" "Home with Ctrl-A" "word1" 'expect << '\''EOF'\''
spawn bash -c "echo '\''word1 word2 word3 word4'\'' | ./test_yank_t12"
set timeout 5
expect -re ".*" {
    send "l"
    sleep 0.2
    send "l"
    sleep 0.2
    send "\001"
    sleep 0.2
    send "\r"
    expect eof
}
EOF' "test_yank_t12"

# Test 13: End with Ctrl-E
run_case "13" "End with Ctrl-E" "word4" 'expect << '\''EOF'\''
spawn bash -c "echo '\''word1 word2 word3 word4'\'' | ./test_yank_t13"
set timeout 5
expect -re ".*" {
    send "\005"
    sleep 0.2
    send "\r"
    expect eof
}
EOF' "test_yank_t13"

# Test 14: Exit with Ctrl-C
run_case "14" "Exit with Ctrl-C" "EXIT_WITHOUT_CLIPBOARD" 'expect << '\''EOF'\''
spawn bash -c "echo '\''test data'\'' | ./test_yank_t14"
set timeout 5
expect -re ".*" {
    send "\003"
    expect eof
}
EOF' "test_yank_t14"

# Test 15: Exit with Ctrl-D
run_case "15" "Exit with Ctrl-D" "EXIT_WITHOUT_CLIPBOARD" 'expect << '\''EOF'\''
spawn bash -c "echo '\''test data'\'' | ./test_yank_t15"
set timeout 5
expect -re ".*" {
    send "\004"
    expect eof
}
EOF' "test_yank_t15"

# Test 16: Long text
run_case "16" "Long text" "very_long_word_that_might_exceed_terminal_width_and_cause_wrapping_issues_in_the_display_logic" 'expect << '\''EOF'\''
spawn bash -c "echo '\''very_long_word_that_might_exceed_terminal_width_and_cause_wrapping_issues_in_the_display_logic'\'' | ./test_yank_t16"
set timeout 5
expect -re ".*" {
    send "\r"
    expect eof
}
EOF' "test_yank_t16"

# Test 17: Empty input
run_case "17" "Empty input" "" 'expect << '\''EOF'\''
spawn bash -c "echo '\'''\'' | ./test_yank_t17"
set timeout 5
expect -re ".*" {
    send "\r"
    expect eof
}
EOF' "test_yank_t17"

# Test 18: Input with special characters
run_case "18" "Special characters" "test@example.com" 'expect << '\''EOF'\''
spawn bash -c "echo '\''test@example.com http://example.com /path/to/file'\'' | ./test_yank_t18"
set timeout 5
expect -re ".*" {
    send "\r"
    expect eof
}
EOF' "test_yank_t18"

# Test 19: Custom delimiter (-d)
run_case "19" "Custom delimiter (-d)" "word1" 'expect << EOF
spawn bash -c "echo \"word1:word2:word3\" | ./test_yank_t19 -d \":\""
set timeout 5
expect -re ".*" {
    send "\r"
    expect eof
}
EOF' "test_yank_t19"

# Test 20: Line mode (-l)
run_case "20" "Line mode (-l)" "line1" 'expect << EOF
spawn bash -c "printf \"line1\\nline2\\nline3\" | ./test_yank_t20 -l"
set timeout 5
expect -re ".*" {
    send "\r"
    expect eof
}
EOF' "test_yank_t20"

# Test 21: Case insensitive (-i)
run_case "21" "Case insensitive (-i)" "Word1" 'expect << EOF
spawn bash -c "echo \"Word1 WORD2 word3\" | ./test_yank_t21 -i"
set timeout 5
expect -re ".*" {
    send "\r"
    expect eof
}
EOF' "test_yank_t21"

# Test 22: Alternate screen mode (-x)
run_case "22" "Alternate screen mode (-x)" "test" 'expect << EOF
spawn bash -c "echo \"test data\" | ./test_yank_t22 -x"
set timeout 5
expect -re ".*" {
    send "\r"
    expect eof
}
EOF' "test_yank_t22"

# Test 23: Version display (-v)
echo "Test 23 started"
log_tmp="flow_results/test23_tmp.log"
: > "$log_tmp"
LD_PRELOAD=libtracer.so TRACE_OUTPUT="$PWD/flow_results/test23_trace.log" \
    ./test_yank_t23 -v >> "$log_tmp" 2>&1
rc=$?
if [ $rc -eq 0 ]; then
    echo "Test 23 passed"
    mv "$log_tmp" flow_results/test23_success.log
else
    echo "Test 23 failed" >&2
    echo "Test 23 failed"
    failed=1
    mv "$log_tmp" flow_results/test23_fail.log
fi
echo "Test 23 ended"

# Test 24: Help display (invalid option should fail)
echo "Test 24 started"
log_tmp="flow_results/test24_tmp.log"
: > "$log_tmp"
LD_PRELOAD=libtracer.so TRACE_OUTPUT="$PWD/flow_results/test24_trace.log" \
    ./test_yank_t24 -h >> "$log_tmp" 2>&1
rc=$?
if [ $rc -ne 0 ]; then
    echo "Test 24 passed"
    mv "$log_tmp" flow_results/test24_success.log
else
    echo "Test 24 failed" >&2
    echo "Test 24 failed"
    failed=1
    mv "$log_tmp" flow_results/test24_fail.log
fi
echo "Test 24 ended"

# Cleanup
rm -f /tmp/yank_clipboard xsel

exit $failed

