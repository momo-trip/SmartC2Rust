#!/bin/bash

for ((current_test=1; current_test<=5; current_test++)); do
    TEST_NUM=$current_test
    ACTUAL_OUTPUT=$(./test $TEST_NUM 2>&1)

    case $TEST_NUM in
        1)
            # Test 1: Named colors
            EXPECTED_OUTPUT="Running test 1: Named colors
Value: 1
Parse status: OK
Expected: -2139094785, Actual: -2139094785 - PASS"
            ;;
        2)
            # Test 2: RGB format
            EXPECTED_OUTPUT="Running test 2: RGB format
Value: 1
Parse status: OK
Expected: -14810881, Actual: -14810881 - PASS
Value: 1
Parse status: OK
Expected: 255, Actual: 255 - PASS"
            ;;
        3)
            # Test 3: RGBA format
            EXPECTED_OUTPUT="Running test 3: RGBA format
Value: 1
Parse status: OK
Expected: -14811009, Actual: -14811009 - PASS
Value: 1
Parse status: OK
Expected: 255, Actual: 255 - PASS"
            ;;
        4)
            # Test 4: Hex format
            EXPECTED_OUTPUT="Running test 4: Hex format
Value: 1
Parse status: OK
Expected: -14810881, Actual: -14810881 - PASS
Value: 1
Parse status: OK
Expected: -1, Actual: -1 - PASS
Value: 1
Parse status: OK
Expected: -3407617, Actual: -3407617 - PASS
Value: 1
Parse status: OK
Expected: -3407617, Actual: -3407617 - PASS"
            ;;
        5)
            # Test 5: To string conversion
            EXPECTED_OUTPUT="Running test 5: To string conversion
Expected: 0, Actual: 0 - PASS
Expected: 0, Actual: 0 - PASS"
            ;;
        *)
            echo "Error: Invalid test number $TEST_NUM"
            echo "Valid test numbers: 1-5"
            exit 1
            ;;
    esac

    echo "Running test $TEST_NUM..."
    echo "----------------------------------------"

    echo "Expected length: ${#EXPECTED_OUTPUT}"
    echo "Actual length: ${#ACTUAL_OUTPUT}"
    echo "----------------------------------------"

    if [ "$EXPECTED_OUTPUT" = "$ACTUAL_OUTPUT" ]; then
        echo -e "\e[32m✓ Test $TEST_NUM PASSED - Output matches expected result\e[0m"
    else
        echo -e "\e[31m✗ Test $TEST_NUM FAILED - Output differs from expected result\e[0m"
        echo ""
        echo "=== EXPECTED OUTPUT ==="
        echo "$EXPECTED_OUTPUT"
        echo ""
        echo "=== ACTUAL OUTPUT ==="
        echo "$ACTUAL_OUTPUT"
        echo ""
        

        echo "=== DETAILED COMPARISON ==="
        echo "Hex dump of expected (first 200 bytes):"
        echo "$EXPECTED_OUTPUT" | xxd | head -20
        echo ""
        echo "Hex dump of actual (first 200 bytes):"
        echo "$ACTUAL_OUTPUT" | xxd | head -20
        echo ""
        
        echo "=== DIFF ==="
        diff <(echo "$EXPECTED_OUTPUT") <(echo "$ACTUAL_OUTPUT") || true

    fi
    
    echo ""
    echo "========================================"
    echo ""
done

echo "All tests completed."