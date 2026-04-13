# Reformat test cases (Step 1)

## Overview

Step 1 reformats the original test script (`base_test.sh`) into a standardized format (`run_test.sh`) so that the subsequent iterative cycle can run automatically. The LLM is used to analyze the original test script and generate the reformatted version.

## Why reformatting is necessary

The original `base_test.sh` is written for human use and varies in style across benchmarks. The pipeline requires a standardized test format where:

- Each test case is isolated and independently runnable
- Each test invokes a per-test binary with tracing instrumentation
- Results are logged individually for golden flow extraction and semantic repair

## What changes

### Per-test binary

The original script calls a single binary (e.g., `./build/zopfli`). The reformatted script calls a per-test binary (e.g., `./test_binaries/zopfli_t1`, `./test_binaries/zopfli_t2`, ...). 
This allows golden flows to be recorded separately for each test case, so that the LLM can localize and fix failures on a per-test basis.

### Tracing instrumentation

Each test is executed with `LD_PRELOAD=libtracer.so` and `TRACE_OUTPUT` set to a per-test trace log. This captures the execution flow of the C program for each test case.

```bash
# Before (base_test.sh)
./build/zopfli test_input.txt

# After (run_test.sh)
LD_PRELOAD=libtracer.so TRACE_OUTPUT=$PWD/flow_results/test1_trace.log ./test_binaries/zopfli_t1 test_input.txt
```

### Standardized pass/fail reporting

The original script may use custom assertion functions, colored output, or helper functions. The reformatted script uses a uniform format:

```
Test N started
Test N passed    # or: Test N failed
Test N ended
```

Failed tests also write to stderr (`echo "Test N failed" >&2`).

### Individual result logging

Each test writes its output to `flow_results/testN_success.log` or `flow_results/testN_fail.log`, enabling the pipeline to inspect results per test case.

## Command

```bash
cd /root/SmartC2Rust/macro
python3 pre_process.py /root/SmartC2Rust/benchmark/{program} reformat base /root/SmartC2Rust/benchmark/{program}/base_test.sh
```

## Input

- `benchmark/{program}/base_test.sh` — the original test script

## Output

- `benchmark/{program}/run_test.sh` — the reformatted test script
- `macro/chats_0000_reformat/{program}/` — LLM interaction prompt logs