#!/bin/bash

# Organized test suite for mptre/yank

echo "=== Setting up xsel mock ==="
# Create xsel mock to avoid clipboard errors
cat > xsel << 'EOF'
#!/bin/bash
# Output to file instead of xsel
cat > /tmp/yank_clipboard
echo "Content saved to clipboard (mock)" >&2
EOF

chmod +x xsel
export PATH="$(pwd):$PATH"

echo "=== Running Organized Tests ==="

# Check if expect is available
if ! command -v expect &> /dev/null; then
    echo "ERROR: expect command not found. Install with: sudo apt-get install expect"
    exit 1
fi

# Test result counters
passed=0
failed=0
total=0

# Test execution function
run_test() {
    local test_num="$1"
    local test_name="$2"
    local expected="$3"
    local expect_script="$4"
    
    echo "Test $test_num: $test_name"
    echo "Expected: '$expected'"
    
    total=$((total + 1))
    
    # Execute expect script with complete output suppression
    {
        eval "$expect_script"
    } > /dev/null 2>&1
    
    # Get result
    local actual
    if [ "$expected" = "EXIT_WITHOUT_CLIPBOARD" ]; then
        if [ ! -f /tmp/yank_clipboard ]; then
            actual="EXIT_WITHOUT_CLIPBOARD"
        else
            actual="$(cat /tmp/yank_clipboard 2>/dev/null || echo '')"
        fi
    else
        actual="$(cat /tmp/yank_clipboard 2>/dev/null || echo '')"
    fi
    
    echo "Actual: '$actual'"
    
    if [ "$actual" = "$expected" ]; then
        echo "✓ PASS"
        passed=$((passed + 1))
    else
        echo "✗ FAIL"
        failed=$((failed + 1))
    fi
    
    echo ""
    rm -f /tmp/yank_clipboard
}

# === Homebrew test case ===
run_test "01" "Homebrew test case (key=value, select value)" "value" 'expect << EOF
spawn bash -c "echo \"key=value\" | ./yank -d \"=\""
set timeout 5
expect -re ".*" { 
    send "\016"
    sleep 0.2
    send "\r"
    expect eof 
}
EOF'

# === Basic selection tests ===
run_test "02" "Basic selection (first word)" "hello" 'expect << '\''EOF'\''
spawn bash -c "echo '\''hello world test data'\'' | ./yank"
set timeout 5
expect -re ".*" { send "\r"; expect eof }
EOF'

run_test "03" "Multi-line selection (first line first word)" "line1" 'expect << '\''EOF'\''
spawn bash -c "echo -e '\''line1\\nline2\\nline3'\'' | ./yank"
set timeout 5
expect -re ".*" { send "\r"; expect eof }
EOF'

run_test "04" "Word selection (first word)" "word1" 'expect << '\''EOF'\''
spawn bash -c "echo '\''word1 word2 word3'\'' | ./yank"
set timeout 5
expect -re ".*" { send "\r"; expect eof }
EOF'

# === Keyboard navigation tests ===
run_test "05" "Right navigation (l key)" "word2" 'expect << '\''EOF'\''
spawn bash -c "echo '\''word1 word2 word3'\'' | ./yank"
set timeout 5
expect -re ".*" { 
    send "l"
    sleep 0.2
    send "\r"
    expect eof 
}
EOF'

run_test "06" "Left navigation (h key)" "word1" 'expect << '\''EOF'\''
spawn bash -c "echo '\''word1 word2 word3'\'' | ./yank"
set timeout 5
expect -re ".*" { 
    send "l"
    sleep 0.2
    send "h"
    sleep 0.2
    send "\r"
    expect eof 
}
EOF'

run_test "07" "Go to end (G key)" "fourth" 'expect << '\''EOF'\''
spawn bash -c "echo '\''first second third fourth'\'' | ./yank"
set timeout 5
expect -re ".*" { 
    send "G"
    sleep 0.2
    send "\r"
    expect eof 
}
EOF'

run_test "08" "Go to beginning (g key)" "first" 'expect << '\''EOF'\''
spawn bash -c "echo '\''first second third fourth'\'' | ./yank"
set timeout 5
expect -re ".*" { 
    send "G"
    sleep 0.2
    send "g"
    sleep 0.2
    send "\r"
    expect eof 
}
EOF'

run_test "09" "Down navigation in multi-line (j key)" "line2" 'expect << '\''EOF'\''
spawn bash -c "printf '\''line1 word1 word2\nline2 word3 word4\nline3 word5 word6'\'' | ./yank"
set timeout 5
expect -re ".*" { 
    send "j"
    sleep 0.2
    send "\r"
    expect eof 
}
EOF'

run_test "10" "Up navigation in multi-line (k key)" "line1" 'expect << '\''EOF'\''
spawn bash -c "printf '\''line1 word1 word2\nline2 word3 word4\nline3 word5 word6'\'' | ./yank"
set timeout 5
expect -re ".*" { 
    send "j"
    sleep 0.2
    send "k"
    sleep 0.2
    send "\r"
    expect eof 
}
EOF'

run_test "11" "Arrow key navigation (right)" "word2" 'expect << '\''EOF'\''
spawn bash -c "echo '\''word1 word2 word3'\'' | ./yank"
set timeout 5
expect -re ".*" { 
    send "\033\[C"
    sleep 0.2
    send "\r"
    expect eof 
}
EOF'

# === Control key tests ===
run_test "12" "Home with Ctrl-A" "word1" 'expect << '\''EOF'\''
spawn bash -c "echo '\''word1 word2 word3 word4'\'' | ./yank"
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
EOF'

run_test "13" "End with Ctrl-E" "word4" 'expect << '\''EOF'\''
spawn bash -c "echo '\''word1 word2 word3 word4'\'' | ./yank"
set timeout 5
expect -re ".*" { 
    send "\005"
    sleep 0.2
    send "\r"
    expect eof 
}
EOF'

# === Exit tests ===
run_test "14" "Exit with Ctrl-C" "EXIT_WITHOUT_CLIPBOARD" 'expect << '\''EOF'\''
spawn bash -c "echo '\''test data'\'' | ./yank"
set timeout 5
expect -re ".*" { 
    send "\003"
    expect eof 
}
EOF'

run_test "15" "Exit with Ctrl-D" "EXIT_WITHOUT_CLIPBOARD" 'expect << '\''EOF'\''
spawn bash -c "echo '\''test data'\'' | ./yank"
set timeout 5
expect -re ".*" { 
    send "\004"
    expect eof 
}
EOF'

# === Edge case tests ===
run_test "16" "Long text exceeding terminal width" "very_long_word_that_might_exceed_terminal_width_and_cause_wrapping_issues_in_the_display_logic" 'expect << '\''EOF'\''
spawn bash -c "echo '\''very_long_word_that_might_exceed_terminal_width_and_cause_wrapping_issues_in_the_display_logic'\'' | ./yank"
set timeout 5
expect -re ".*" { 
    send "\r"
    expect eof 
}
EOF'

run_test "17" "Empty input" "" 'expect << '\''EOF'\''
spawn bash -c "echo '\'''\'' | ./yank"
set timeout 5
expect -re ".*" { 
    send "\r"
    expect eof 
}
EOF'

run_test "18" "Input with special characters" "test@example.com" 'expect << '\''EOF'\''
spawn bash -c "echo '\''test@example.com http://example.com /path/to/file'\'' | ./yank"
set timeout 5
expect -re ".*" { 
    send "\r"
    expect eof 
}
EOF'

# === Command line option tests ===
run_test "19" "Custom delimiter (-d)" "word1" 'expect << EOF
spawn bash -c "echo \"word1:word2:word3\" | ./yank -d \":\""
set timeout 5
expect -re ".*" { 
    send "\r"
    expect eof 
}
EOF'

run_test "20" "Line mode (-l)" "line1" 'expect << EOF
spawn bash -c "printf \"line1\\nline2\\nline3\" | ./yank -l"
set timeout 5
expect -re ".*" { 
    send "\r"
    expect eof 
}
EOF'

run_test "21" "Case insensitive (-i)" "Word1" 'expect << EOF
spawn bash -c "echo \"Word1 WORD2 word3\" | ./yank -i"
set timeout 5
expect -re ".*" { 
    send "\r"
    expect eof 
}
EOF'

run_test "22" "Alternate screen mode (-x)" "test" 'expect << EOF
spawn bash -c "echo \"test data\" | ./yank -x"
set timeout 5
expect -re ".*" { 
    send "\r"
    expect eof 
}
EOF'

# === Special tests ===
echo "=== Special Tests ==="

echo "Test 23: Version display (-v)"
./yank -v 2>/dev/null && echo "✓ PASS: Version displayed" || echo "✗ FAIL: Version not displayed"
echo ""

echo "Test 24: Help display (invalid option)"
./yank -h 2>/dev/null && echo "✗ FAIL: Should show error" || echo "✓ PASS: Error shown as expected"
echo ""

# Cleanup
rm -f /tmp/yank_clipboard xsel

echo "=== Test Summary ==="
echo "Total tests: $total"
echo "Passed: $passed"
echo "Failed: $failed"
echo "Success rate: $(( passed * 100 / total ))%"

if [ $failed -eq 0 ]; then
    echo "🎉 All tests passed!"
    exit 0
else
    echo "❌ Some tests failed!"
    exit 1
fi

# === 基本選択テスト ===
run_test "01" "Basic selection (first word)" "hello" 'expect << '\''EOF'\''
spawn bash -c "echo '\''hello world test data'\'' | ./yank"
set timeout 5
expect -re ".*" { send "\r"; expect eof }
EOF'

run_test "02" "Multi-line selection (first line first word)" "line1" 'expect << '\''EOF'\''
spawn bash -c "echo -e '\''line1\\nline2\\nline3'\'' | ./yank"
set timeout 5
expect -re ".*" { send "\r"; expect eof }
EOF'

run_test "03" "Word selection (first word)" "word1" 'expect << '\''EOF'\''
spawn bash -c "echo '\''word1 word2 word3'\'' | ./yank"
set timeout 5
expect -re ".*" { send "\r"; expect eof }
EOF'

# === キーボードナビゲーションテスト ===
run_test "04" "Right navigation (l key)" "word2" 'expect << '\''EOF'\''
spawn bash -c "echo '\''word1 word2 word3'\'' | ./yank"
set timeout 5
expect -re ".*" { 
    send "l"
    sleep 0.2
    send "\r"
    expect eof 
}
EOF'

run_test "05" "Left navigation (h key)" "word1" 'expect << '\''EOF'\''
spawn bash -c "echo '\''word1 word2 word3'\'' | ./yank"
set timeout 5
expect -re ".*" { 
    send "l"
    sleep 0.2
    send "h"
    sleep 0.2
    send "\r"
    expect eof 
}
EOF'

run_test "06" "Go to end (G key)" "fourth" 'expect << '\''EOF'\''
spawn bash -c "echo '\''first second third fourth'\'' | ./yank"
set timeout 5
expect -re ".*" { 
    send "G"
    sleep 0.2
    send "\r"
    expect eof 
}
EOF'

run_test "07" "Go to beginning (g key)" "first" 'expect << '\''EOF'\''
spawn bash -c "echo '\''first second third fourth'\'' | ./yank"
set timeout 5
expect -re ".*" { 
    send "G"
    sleep 0.2
    send "g"
    sleep 0.2
    send "\r"
    expect eof 
}
EOF'

run_test "08" "Down navigation in multi-line (j key)" "line2" 'expect << '\''EOF'\''
spawn bash -c "printf '\''line1 word1 word2\nline2 word3 word4\nline3 word5 word6'\'' | ./yank"
set timeout 5
expect -re ".*" { 
    send "j"
    sleep 0.2
    send "\r"
    expect eof 
}
EOF'

run_test "09" "Up navigation in multi-line (k key)" "line1" 'expect << '\''EOF'\''
spawn bash -c "printf '\''line1 word1 word2\nline2 word3 word4\nline3 word5 word6'\'' | ./yank"
set timeout 5
expect -re ".*" { 
    send "j"
    sleep 0.2
    send "k"
    sleep 0.2
    send "\r"
    expect eof 
}
EOF'

run_test "10" "Arrow key navigation (right)" "word2" 'expect << '\''EOF'\''
spawn bash -c "echo '\''word1 word2 word3'\'' | ./yank"
set timeout 5
expect -re ".*" { 
    send "\033\[C"
    sleep 0.2
    send "\r"
    expect eof 
}
EOF'

# === 制御キーテスト ===
run_test "11" "Home with Ctrl-A" "word1" 'expect << '\''EOF'\''
spawn bash -c "echo '\''word1 word2 word3 word4'\'' | ./yank"
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
EOF'

run_test "12" "End with Ctrl-E" "word4" 'expect << '\''EOF'\''
spawn bash -c "echo '\''word1 word2 word3 word4'\'' | ./yank"
set timeout 5
expect -re ".*" { 
    send "\005"
    sleep 0.2
    send "\r"
    expect eof 
}
EOF'

# === 終了テスト ===
run_test "13" "Exit with Ctrl-C" "EXIT_WITHOUT_CLIPBOARD" 'expect << '\''EOF'\''
spawn bash -c "echo '\''test data'\'' | ./yank"
set timeout 5
expect -re ".*" { 
    send "\003"
    expect eof 
}
EOF'

run_test "14" "Exit with Ctrl-D" "EXIT_WITHOUT_CLIPBOARD" 'expect << '\''EOF'\''
spawn bash -c "echo '\''test data'\'' | ./yank"
set timeout 5
expect -re ".*" { 
    send "\004"
    expect eof 
}
EOF'

# === エッジケーステスト ===
run_test "15" "Long text exceeding terminal width" "very_long_word_that_might_exceed_terminal_width_and_cause_wrapping_issues_in_the_display_logic" 'expect << '\''EOF'\''
spawn bash -c "echo '\''very_long_word_that_might_exceed_terminal_width_and_cause_wrapping_issues_in_the_display_logic'\'' | ./yank"
set timeout 5
expect -re ".*" { 
    send "\r"
    expect eof 
}
EOF'

run_test "16" "Empty input" "" 'expect << '\''EOF'\''
spawn bash -c "echo '\'''\'' | ./yank"
set timeout 5
expect -re ".*" { 
    send "\r"
    expect eof 
}
EOF'

run_test "17" "Input with special characters" "test@example.com" 'expect << '\''EOF'\''
spawn bash -c "echo '\''test@example.com http://example.com /path/to/file'\'' | ./yank"
set timeout 5
expect -re ".*" { 
    send "\r"
    expect eof 
}
EOF'

# === コマンドラインオプションテスト ===
run_test "18" "Custom delimiter (-d)" "word1" 'expect << EOF
spawn bash -c "echo \"word1:word2:word3\" | ./yank -d \":\""
set timeout 5
expect -re ".*" { 
    send "\r"
    expect eof 
}
EOF'

run_test "19" "Line mode (-l)" "line1" 'expect << EOF
spawn bash -c "printf \"line1\\nline2\\nline3\" | ./yank -l"
set timeout 5
expect -re ".*" { 
    send "\r"
    expect eof 
}
EOF'

run_test "20" "Case insensitive (-i)" "Word1" 'expect << EOF
spawn bash -c "echo \"Word1 WORD2 word3\" | ./yank -i"
set timeout 5
expect -re ".*" { 
    send "\r"
    expect eof 
}
EOF'

run_test "21" "Alternate screen mode (-x)" "test" 'expect << EOF
spawn bash -c "echo \"test data\" | ./yank -x"
set timeout 5
expect -re ".*" { 
    send "\r"
    expect eof 
}
EOF'

# === Homebrewテストケース ===
run_test "22" "Homebrew test case (key=value, select value)" "value" 'expect << EOF
spawn bash -c "echo \"key=value\" | ./yank -d \"=\""
set timeout 5
expect -re ".*" { 
    send "\016"
    sleep 0.2
    send "\r"
    expect eof 
}
EOF'

# === 特殊テスト ===
echo "=== Special Tests ==="

echo "Test 23: Version display (-v)"
./yank -v 2>/dev/null && echo "✓ PASS: Version displayed" || echo "✗ FAIL: Version not displayed"
echo ""

echo "Test 24: Help display (invalid option)"
./yank -h 2>/dev/null && echo "✗ FAIL: Should show error" || echo "✓ PASS: Error shown as expected"
echo ""

# Cleanup
rm -f /tmp/yank_clipboard xsel

echo "=== Test Summary ==="
echo "Total tests: $total"
echo "Passed: $passed"
echo "Failed: $failed"
echo "Success rate: $(( passed * 100 / total ))%"

if [ $failed -eq 0 ]; then
    echo "🎉 All tests passed!"
    exit 0
else
    echo "❌ Some tests failed!"
    exit 1
fi