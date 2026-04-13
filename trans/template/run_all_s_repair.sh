#!/bin/bash
set -e

# Build the Rust library
cd trans_rust
./rust_build.sh

# Build the C library
cd ../{target_name}
./c_build.sh

# Run the test
./run_test.sh