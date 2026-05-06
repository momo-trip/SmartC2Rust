#!/bin/bash

# get_expected.sh - Generate expected output files for markandsweep tests

# Create expected directory
echo "Creating expected directory..."
rm -rf expected
mkdir -p expected

# Check if markandsweep executable exists
if [ ! -f ./markandsweep ]; then
    echo "Error: markandsweep executable not found!" >&2
    echo "Please build the project first." >&2
    exit 1
fi

# Function to generate expected output for a test case
generate_expected() {
    local test_num=$1
    local output_file="expected/test${test_num}.log"
    
    echo "Generating expected output for test case ${test_num}..."
    
    # Run the test and capture output
    ./markandsweep $test_num > "$output_file" 2>&1
    
    # Check if the test ran successfully
    if [ $? -eq 0 ]; then
        echo "✓ Test ${test_num} expected output saved to ${output_file}"
        # Show first few lines of output
        echo "  Preview (first 5 lines):"
        head -5 "$output_file" | sed 's/^/    /'
    else
        echo "✗ Test ${test_num} failed to run" >&2
        echo "  Error output saved to ${output_file}"
    fi
    echo ""
}

# Generate expected outputs for all test cases
echo "Generating expected outputs for all test cases..."
echo "================================================="

for i in {1..5}; do
    generate_expected $i
done

echo "Expected output generation complete!"
echo ""
echo "Generated files:"
ls -la expected/
