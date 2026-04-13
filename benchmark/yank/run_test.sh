
#!/bin/bash

# Reformed test cases for yank

failed=0

mkdir -p flow_results

# Create xsel mock to avoid clipboard errors
cat > xsel << 'MOCKEOF'
#!/bin/bash
# Output to file instead of xsel
cat > /tmp/yank_clipboard
echo "Content saved to clipboard (mock)" >&2
MOCKEOF
chmod +x xsel
export PATH="$(pwd):$PATH"

# Check if expect is available
if ! command -v expect &> /dev/null; then
    echo "ERROR: expect command not found. Install with: sudo apt-get install expect" >&2
    exit 1
fi

###############################################################################
# Test 1: Homebrew test case (key=value, select value)
###############################################################################
echo "Test 1 started"
rm -f /tmp/yank_clipboard
log=""
expect << 'TESTEOF' > /dev/null 2>&1
spawn bash -c "echo \"key=value\" | ./yank_t1 -d \"=\""
set timeout 5
expect -re ".*" {
    send "\016"
    sleep 0.2
    send "\r"
    expect eof
}
TESTEOF
expected="value"
actual="$(cat /tmp/yank_clipboard 2>/dev/null)"
if [ "$actual" = "$expected" ]; then
    echo "Test 1 passed"
    echo "Test 1: expected='$expected' actual='$actual' PASSED" > flow_results/test1_success.log
else
    echo "Test 1 failed"
    echo "Test 1 failed" >&2
    echo "Test 1: expected='$expected' actual='$actual' FAILED" > flow_results/test1_fail.log
    failed=1
fi
echo "Test 1 ended"

###############################################################################
# Test 2: Basic selection (first word)
###############################################################################
echo "Test 2 started"
rm -f /tmp/yank_clipboard
expect << 'TESTEOF' > /dev/null 2>&1
spawn bash -c "echo 'hello world test data' | ./yank_t2"
set timeout 5
expect -re ".*" { send "\r"; expect eof }
TESTEOF
expected="hello"
actual="$(cat /tmp/yank_clipboard 2>/dev/null)"
if [ "$actual" = "$expected" ]; then
    echo "Test 2 passed"
    echo "Test 2: expected='$expected' actual='$actual' PASSED" > flow_results/test2_success.log
else
    echo "Test 2 failed"
    echo "Test 2 failed" >&2
    echo "Test 2: expected='$expected' actual='$actual' FAILED" > flow_results/test2_fail.log
    failed=1
fi
echo "Test 2 ended"

###############################################################################
# Test 3: Multi-line selection (first line first word)
###############################################################################
echo "Test 3 started"
rm -f /tmp/yank_clipboard
expect << 'TESTEOF' > /dev/null 2>&1
spawn bash -c "echo -e 'line1\nline2\nline3' | ./yank_t3"
set timeout 5
expect -re ".*" { send "\r"; expect eof }
TESTEOF
expected="line1"
actual="$(cat /tmp/yank_clipboard 2>/dev/null)"
if [ "$actual" = "$expected" ]; then
    echo "Test 3 passed"
    echo "Test 3: expected='$expected' actual='$actual' PASSED" > flow_results/test3_success.log
else
    echo "Test 3 failed"
    echo "Test 3 failed" >&2
    echo "Test 3: expected='$expected' actual='$actual' FAILED" > flow_results/test3_fail.log
    failed=1
fi
echo "Test 3 ended"

###############################################################################
# Test 4: Word selection (first word)
###############################################################################
echo "Test 4 started"
rm -f /tmp/yank_clipboard
expect << 'TESTEOF' > /dev/null 2>&1
spawn bash -c "echo 'word1 word2 word3' | ./yank_t4"
set timeout 5
expect -re ".*" { send "\r"; expect eof }
TESTEOF
expected="word1"
actual="$(cat /tmp/yank_clipboard 2>/dev/null)"
if [ "$actual" = "$expected" ]; then
    echo "Test 4 passed"
    echo "Test 4: expected='$expected' actual='$actual' PASSED" > flow_results/test4_success.log
else
    echo "Test 4 failed"
    echo "Test 4 failed" >&2
    echo "Test 4: expected='$expected' actual='$actual' FAILED" > flow_results/test4_fail.log
    failed=1
fi
echo "Test 4 ended"

###############################################################################
# Test 5: Right navigation (l key)
###############################################################################
echo "Test 5 started"
rm -f /tmp/yank_clipboard
expect << 'TESTEOF' > /dev/null 2>&1
spawn bash -c "echo 'word1 word2 word3' | ./yank_t5"
set timeout 5
expect -re ".*" {
    send "l"
    sleep 0.2
    send "\r"
    expect eof
}
TESTEOF
expected="word2"
actual="$(cat /tmp/yank_clipboard 2>/dev/null)"
if [ "$actual" = "$expected" ]; then
    echo "Test 5 passed"
    echo "Test 5: expected='$expected' actual='$actual' PASSED" > flow_results/test5_success.log
else
    echo "Test 5 failed"
    echo "Test 5 failed" >&2
    echo "Test 5: expected='$expected' actual='$actual' FAILED" > flow_results/test5_fail.log
    failed=1
fi
echo "Test 5 ended"

###############################################################################
# Test 6: Left navigation (h key)
###############################################################################
echo "Test 6 started"
rm -f /tmp/yank_clipboard
expect << 'TESTEOF' > /dev/null 2>&1
spawn bash -c "echo 'word1 word2 word3' | ./yank_t6"
set timeout 5
expect -re ".*" {
    send "l"
    sleep 0.2
    send "h"
    sleep 0.2
    send "\r"
    expect eof
}
TESTEOF
expected="word1"
actual="$(cat /tmp/yank_clipboard 2>/dev/null)"
if [ "$actual" = "$expected" ]; then
    echo "Test 6 passed"
    echo "Test 6: expected='$expected' actual='$actual' PASSED" > flow_results/test6_success.log
else
    echo "Test 6 failed"
    echo "Test 6 failed" >&2
    echo "Test 6: expected='$expected' actual='$actual' FAILED" > flow_results/test6_fail.log
    failed=1
fi
echo "Test 6 ended"

###############################################################################
# Test 7: Go to end (G key)
###############################################################################
echo "Test 7 started"
rm -f /tmp/yank_clipboard
expect << 'TESTEOF' > /dev/null 2>&1
spawn bash -c "echo 'first second third fourth' | ./yank_t7"
set timeout 5
expect -re ".*" {
    send "G"
    sleep 0.2
    send "\r"
    expect eof
}
TESTEOF
expected="fourth"
actual="$(cat /tmp/yank_clipboard 2>/dev/null)"
if [ "$actual" = "$expected" ]; then
    echo "Test 7 passed"
    echo "Test 7: expected='$expected' actual='$actual' PASSED" > flow_results/test7_success.log
else
    echo "Test 7 failed"
    echo "Test 7 failed" >&2
    echo "Test 7: expected='$expected' actual='$actual' FAILED" > flow_results/test7_fail.log
    failed=1
fi
echo "Test 7 ended"

###############################################################################
# Test 8: Go to beginning (g key)
###############################################################################
echo "Test 8 started"
rm -f /tmp/yank_clipboard
expect << 'TESTEOF' > /dev/null 2>&1
spawn bash -c "echo 'first second third fourth' | ./yank_t8"
set timeout 5
expect -re ".*" {
    send "G"
    sleep 0.2
    send "g"
    sleep 0.2
    send "\r"
    expect eof
}
TESTEOF
expected="first"
actual="$(cat /tmp/yank_clipboard 2>/dev/null)"
if [ "$actual" = "$expected" ]; then
    echo "Test 8 passed"
    echo "Test 8: expected='$expected' actual='$actual' PASSED" > flow_results/test8_success.log
else
    echo "Test 8 failed"
    echo "Test 8 failed" >&2
    echo "Test 8: expected='$expected' actual='$actual' FAILED" > flow_results/test8_fail.log
    failed=1
fi
echo "Test 8 ended"

###############################################################################
# Test 9: Down navigation in multi-line (j key)
###############################################################################
echo "Test 9 started"
rm -f /tmp/yank_clipboard
expect << 'TESTEOF' > /dev/null 2>&1
spawn bash -c "printf 'line1 word1 word2\nline2 word3 word4\nline3 word5 word6' | ./yank_t9"
set timeout 5
expect -re ".*" {
    send "j"
    sleep 0.2
    send "\r"
    expect eof
}
TESTEOF
expected="line2"
actual="$(cat /tmp/yank_clipboard 2>/dev/null)"
if [ "$actual" = "$expected" ]; then
    echo "Test 9 passed"
    echo "Test 9: expected='$expected' actual='$actual' PASSED" > flow_results/test9_success.log
else
    echo "Test 9 failed"
    echo "Test 9 failed" >&2
    echo "Test 9: expected='$expected' actual='$actual' FAILED" > flow_results/test9_fail.log
    failed=1
fi
echo "Test 9 ended"

###############################################################################
# Test 10: Up navigation in multi-line (k key)
###############################################################################
echo "Test 10 started"
rm -f /tmp/yank_clipboard
expect << 'TESTEOF' > /dev/null 2>&1
spawn bash -c "printf 'line1 word1 word2\nline2 word3 word4\nline3 word5 word6' | ./yank_t10"
set timeout 5
expect -re ".*" {
    send "j"
    sleep 0.2
    send "k"
    sleep 0.2
    send "\r"
    expect eof
}
TESTEOF
expected="line1"
actual="$(cat /tmp/yank_clipboard 2>/dev/null)"
if [ "$actual" = "$expected" ]; then
    echo "Test 10 passed"
    echo "Test 10: expected='$expected' actual='$actual' PASSED" > flow_results/test10_success.log
else
    echo "Test 10 failed"
    echo "Test 10 failed" >&2
    echo "Test 10: expected='$expected' actual='$actual' FAILED" > flow_results/test10_fail.log
    failed=1
fi
echo "Test 10 ended"

###############################################################################
# Test 11: Arrow key navigation (right)
###############################################################################
echo "Test 11 started"
rm -f /tmp/yank_clipboard
expect << 'TESTEOF' > /dev/null 2>&1
spawn bash -c "echo 'word1 word2 word3' | ./yank_t11"
set timeout 5
expect -re ".*" {
    send "\033\[C"
    sleep 0.2
    send "\r"
    expect eof
}
TESTEOF
expected="word2"
actual="$(cat /tmp/yank_clipboard 2>/dev/null)"
if [ "$actual" = "$expected" ]; then
    echo "Test 11 passed"
    echo "Test 11: expected='$expected' actual='$actual' PASSED" > flow_results/test11_success.log
else
    echo "Test 11 failed"
    echo "Test 11 failed" >&2
    echo "Test 11: expected='$expected' actual='$actual' FAILED" > flow_results/test11_fail.log
    failed=1
fi
echo "Test 11 ended"

###############################################################################
# Test 12: Home with Ctrl-A
###############################################################################
echo "Test 12 started"
rm -f /tmp/yank_clipboard
expect << 'TESTEOF' > /dev/null 2>&1
spawn bash -c "echo 'word1 word2 word3 word4' | ./yank_t12"
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
TESTEOF
expected="word1"
actual="$(cat /tmp/yank_clipboard 2>/dev/null)"
if [ "$actual" = "$expected" ]; then
    echo "Test 12 passed"
    echo "Test 12: expected='$expected' actual='$actual' PASSED" > flow_results/test12_success.log
else
    echo "Test 12 failed"
    echo "Test 12 failed" >&2
    echo "Test 12: expected='$expected' actual='$actual' FAILED" > flow_results/test12_fail.log
    failed=1
fi
echo "Test 12 ended"

###############################################################################
# Test 13: End with Ctrl-E
###############################################################################
echo "Test 13 started"
rm -f /tmp/yank_clipboard
expect << 'TESTEOF' > /dev/null 2>&1
spawn bash -c "echo 'word1 word2 word3 word4' | ./yank_t13"
set timeout 5
expect -re ".*" {
    send "\005"
    sleep 0.2
    send "\r"
    expect eof
}
TESTEOF
expected="word4"
actual="$(cat /tmp/yank_clipboard 2>/dev/null)"
if [ "$actual" = "$expected" ]; then
    echo "Test 13 passed"
    echo "Test 13: expected='$expected' actual='$actual' PASSED" > flow_results/test13_success.log
else
    echo "Test 13 failed"
    echo "Test 13 failed" >&2
    echo "Test 13: expected='$expected' actual='$actual' FAILED" > flow_results/test13_fail.log
    failed=1
fi
echo "Test 13 ended"

###############################################################################
# Test 14: Exit with Ctrl-C
###############################################################################
echo "Test 14 started"
rm -f /tmp/yank_clipboard
expect << 'TESTEOF' > /dev/null 2>&1
spawn bash -c "echo 'test data' | ./yank_t14"
set timeout 5
expect -re ".*" {
    send "\003"
    expect eof
}
TESTEOF
expected="EXIT_WITHOUT_CLIPBOARD"
if [ ! -f /tmp/yank_clipboard ]; then
    actual="EXIT_WITHOUT_CLIPBOARD"
else
    actual="$(cat /tmp/yank_clipboard 2>/dev/null)"
fi
if [ "$actual" = "$expected" ]; then
    echo "Test 14 passed"
    echo "Test 14: expected='$expected' actual='$actual' PASSED" > flow_results/test14_success.log
else
    echo "Test 14 failed"
    echo "Test 14 failed" >&2
    echo "Test 14: expected='$expected' actual='$actual' FAILED" > flow_results/test14_fail.log
    failed=1
fi
echo "Test 14 ended"

###############################################################################
# Test 15: Exit with Ctrl-D
###############################################################################
echo "Test 15 started"
rm -f /tmp/yank_clipboard
expect << 'TESTEOF' > /dev/null 2>&1
spawn bash -c "echo 'test data' | ./yank_t15"
set timeout 5
expect -re ".*" {
    send "\004"
    expect eof
}
TESTEOF
expected="EXIT_WITHOUT_CLIPBOARD"
if [ ! -f /tmp/yank_clipboard ]; then
    actual="EXIT_WITHOUT_CLIPBOARD"
else
    actual="$(cat /tmp/yank_clipboard 2>/dev/null)"
fi
if [ "$actual" = "$expected" ]; then
    echo "Test 15 passed"
    echo "Test 15: expected='$expected' actual='$actual' PASSED" > flow_results/test15_success.log
else
    echo "Test 15 failed"
    echo "Test 15 failed" >&2
    echo "Test 15: expected='$expected' actual='$actual' FAILED" > flow_results/test15_fail.log
    failed=1
fi
echo "Test 15 ended"

###############################################################################
# Test 16: Long text exceeding terminal width
###############################################################################
echo "Test 16 started"
rm -f /tmp/yank_clipboard
expect << 'TESTEOF' > /dev/null 2>&1
spawn bash -c "echo 'very_long_word_that_might_exceed_terminal_width_and_cause_wrapping_issues_in_the_display_logic' | ./yank_t16"
set timeout 5
expect -re ".*" {
    send "\r"
    expect eof
}
TESTEOF
expected="very_long_word_that_might_exceed_terminal_width_and_cause_wrapping_issues_in_the_display_logic"
actual="$(cat /tmp/yank_clipboard 2>/dev/null)"
if [ "$actual" = "$expected" ]; then
    echo "Test 16 passed"
    echo "Test 16: expected='$expected' actual='$actual' PASSED" > flow_results/test16_success.log
else
    echo "Test 16 failed"
    echo "Test 16 failed" >&2
    echo "Test 16: expected='$expected' actual='$actual' FAILED" > flow_results/test16_fail.log
    failed=1
fi
echo "Test 16 ended"

###############################################################################
# Test 17: Empty input
###############################################################################
echo "Test 17 started"
rm -f /tmp/yank_clipboard
expect << 'TESTEOF' > /dev/null 2>&1
spawn bash -c "echo '' | ./yank_t17"
set timeout 5
expect -re ".*" {
    send "\r"
    expect eof
}
TESTEOF
expected=""
actual="$(cat /tmp/yank_clipboard 2>/dev/null)"
if [ "$actual" = "$expected" ]; then
    echo "Test 17 passed"
    echo "Test 17: expected='$expected' actual='$actual' PASSED" > flow_results/test17_success.log
else
    echo "Test 17 failed"
    echo "Test 17 failed" >&2
    echo "Test 17: expected='$expected' actual='$actual' FAILED" > flow_results/test17_fail.log
    failed=1
fi
echo "Test 17 ended"

###############################################################################
# Test 18: Input with special characters
###############################################################################
echo "Test 18 started"
rm -f /tmp/yank_clipboard
expect << 'TESTEOF' > /dev/null 2>&1
spawn bash -c "echo 'test@example.com http://example.com /path/to/file' | ./yank_t18"
set timeout 5
expect -re ".*" {
    send "\r"
    expect eof
}
TESTEOF
expected="test@example.com"
actual="$(cat /tmp/yank_clipboard 2>/dev/null)"
if [ "$actual" = "$expected" ]; then
    echo "Test 18 passed"
    echo "Test 18: expected='$expected' actual='$actual' PASSED" > flow_results/test18_success.log
else
    echo "Test 18 failed"
    echo "Test 18 failed" >&2
    echo "Test 18: expected='$expected' actual='$actual' FAILED" > flow_results/test18_fail.log
    failed=1
fi
echo "Test 18 ended"

###############################################################################
# Test 19: Custom delimiter (-d)
###############################################################################
echo "Test 19 started"
rm -f /tmp/yank_clipboard
expect << 'TESTEOF' > /dev/null 2>&1
spawn bash -c "echo \"word1:word2:word3\" | ./yank_t19 -d \":\""
set timeout 5
expect -re ".*" {
    send "\r"
    expect eof
}
TESTEOF
expected="word1"
actual="$(cat /tmp/yank_clipboard 2>/dev/null)"
if [ "$actual" = "$expected" ]; then
    echo "Test 19 passed"
    echo "Test 19: expected='$expected' actual='$actual' PASSED" > flow_results/test19_success.log
else
    echo "Test 19 failed"
    echo "Test 19 failed" >&2
    echo "Test 19: expected='$expected' actual='$actual' FAILED" > flow_results/test19_fail.log
    failed=1
fi
echo "Test 19 ended"

###############################################################################
# Test 20: Line mode (-l)
###############################################################################
echo "Test 20 started"
rm -f /tmp/yank_clipboard
expect << 'TESTEOF' > /dev/null 2>&1
spawn bash -c "printf \"line1\nline2\nline3\" | ./yank_t20 -l"
set timeout 5
expect -re ".*" {
    send "\r"
    expect eof
}
TESTEOF
expected="line1"
actual="$(cat /tmp/yank_clipboard 2>/dev/null)"
if [ "$actual" = "$expected" ]; then
    echo "Test 20 passed"
    echo "Test 20: expected='$expected' actual='$actual' PASSED" > flow_results/test20_success.log
else
    echo "Test 20 failed"
    echo "Test 20 failed" >&2
    echo "Test 20: expected='$expected' actual='$actual' FAILED" > flow_results/test20_fail.log
    failed=1
fi
echo "Test 20 ended"

###############################################################################
# Test 21: Case insensitive (-i)
###############################################################################
echo "Test 21 started"
rm -f /tmp/yank_clipboard
expect << 'TESTEOF' > /dev/null 2>&1
spawn bash -c "echo \"Word1 WORD2 word3\" | ./yank_t21 -i"
set timeout 5
expect -re ".*" {
    send "\r"
    expect eof
}
TESTEOF
expected="Word1"
actual="$(cat /tmp/yank_clipboard 2>/dev/null)"
if [ "$actual" = "$expected" ]; then
    echo "Test 21 passed"
    echo "Test 21: expected='$expected' actual='$actual' PASSED" > flow_results/test21_success.log
else
    echo "Test 21 failed"
    echo "Test 21 failed" >&2
    echo "Test 21: expected='$expected' actual='$actual' FAILED" > flow_results/test21_fail.log
    failed=1
fi
echo "Test 21 ended"

###############################################################################
# Test 22: Alternate screen mode (-x)
###############################################################################
echo "Test 22 started"
rm -f /tmp/yank_clipboard
expect << 'TESTEOF' > /dev/null 2>&1
spawn bash -c "echo \"test data\" | ./yank_t22 -x"
set timeout 5
expect -re ".*" {
    send "\r"
    expect eof
}
TESTEOF
expected="test"
actual="$(cat /tmp/yank_clipboard 2>/dev/null)"
if [ "$actual" = "$expected" ]; then
    echo "Test 22 passed"
    echo "Test 22: expected='$expected' actual='$actual' PASSED" > flow_results/test22_success.log
else
    echo "Test 22 failed"
    echo "Test 22 failed" >&2
    echo "Test 22: expected='$expected' actual='$actual' FAILED" > flow_results/test22_fail.log
    failed=1
fi
echo "Test 22 ended"

###############################################################################
# Test 23: Version display (-v)
###############################################################################
echo "Test 23 started"
rm -f /tmp/yank_clipboard
version_output=$(LD_PRELOAD=libtracer.so TRACE_OUTPUT=$PWD/flow_results/test23_trace.log ./yank_t23 -v 2>&1)
version_rc=$?
if [ $version_rc -eq 0 ]; then
    echo "Test 23 passed"
    echo "Test 23: Version display succeeded. Output: $version_output" > flow_results/test23_success.log
else
    echo "Test 23 failed"
    echo "Test 23 failed" >&2
    echo "Test 23: Version display failed with rc=$version_rc. Output: $version_output" > flow_results/test23_fail.log
    failed=1
fi
echo "Test 23 ended"

###############################################################################
# Test 24: Help display (invalid option)
###############################################################################
echo "Test 24 started"
rm -f /tmp/yank_clipboard
help_output=$(LD_PRELOAD=libtracer.so TRACE_OUTPUT=$PWD/flow_results/test24_trace.log ./yank_t24 -h 2>&1)
help_rc=$?
if [ $help_rc -ne 0 ]; then
    echo "Test 24 passed"
    echo "Test 24: Invalid option correctly returned error. rc=$help_rc Output: $help_output" > flow_results/test24_success.log
else
    echo "Test 24 failed"
    echo "Test 24 failed" >&2
    echo "Test 24: Invalid option should have returned error but rc=$help_rc. Output: $help_output" > flow_results/test24_fail.log
    failed=1
fi
echo "Test 24 ended"

# Cleanup
rm -f /tmp/yank_clipboard xsel

exit $failed

