#!/bin/bash

# Generate expected output for run_test.sh validation.
# This script builds the default AES128 binary and captures its output as the
# golden reference that run_test.sh will compare against (line-prefix match).

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$SCRIPT_DIR"

EXPECTED_DIR="expected"
mkdir -p "${EXPECTED_DIR}"


# Run the binary and capture its output as the expected reference
./test.elf > "${EXPECTED_DIR}/test1_output.log" 2>&1
if [ $? -ne 0 ]; then
    echo "Test binary failed during expected output generation" >&2
    exit 1
fi

echo "Generated ${EXPECTED_DIR}/test1_output.log"