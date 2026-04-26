#!/usr/bin/env bash

failed=0

rm -rf samples/output
mkdir -p samples/output

# Function to run test and verify
run_and_verify() {
    local test_name="$1"
    local output_file="$2"
    local command="$3"
    local filename expected_file
    
    echo "=== Test $test_name ==="
    echo "Running: $command"
    eval "$command"
    local rc=$?
    
    if [ $rc -ne 0 ]; then
        echo "✗ command failed with exit $rc"
        failed=1
        echo ""
        return
    fi

    filename=$(basename "$output_file")
    expected_file="expected/$filename"
    
    if [ -f "$expected_file" ]; then
        echo "Checking $filename..."
        if diff "$expected_file" "$output_file" > /dev/null; then
            echo "✓ $filename matches expected"
        else
            echo "✗ $filename differs from expected:"
            diff "$expected_file" "$output_file"
            failed=1
        fi
    else
        echo "! Expected file $expected_file not found"
        failed=1
    fi
    echo ""
}

# Test 1: lsearch
run_and_verify "1" "samples/output/lsearch.txt" "./lsearch >samples/output/lsearch.txt"

# Test 2: bsearch
run_and_verify "2" "samples/output/bsearch.txt" "./bsearch >samples/output/bsearch.txt"

# Test 3: demo
run_and_verify "3" "samples/output/demo.txt" "echo 'foo bar the bar bar bar the' | ./demo >samples/output/demo.txt"

# Test 4: dump
run_and_verify "4" "samples/output/dump.txt" "./dump >samples/output/dump.txt"

# Generate similar.txt (no verification needed)
echo "=== Generate similar.txt ==="
echo "Running: python3 samples/gensimilar.py 466550 >samples/similar.txt"
python3 samples/gensimilar.py 466550 >samples/similar.txt
echo "✓ similar.txt generated"
echo ""

# Test 5: stats with words.txt
run_and_verify "5" "samples/output/stats-words.txt" "./stats <samples/words.txt >samples/output/stats-words.txt"

# Test 6: stats with similar.txt
run_and_verify "6" "samples/output/stats-similar.txt" "./stats <samples/similar.txt >samples/output/stats-similar.txt"

exit $failed