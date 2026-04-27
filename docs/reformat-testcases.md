# Reformat test cases (Step 1)

## Overview

Step 1 creates a standardized format (`run_test.sh`) so that the subsequent iterative 
cycle can run automatically. You can either write `run_test.sh` 
manually (Option 1) or use the LLM-assisted reformatter (Option 2).

## How the reformatted script is used by the pipeline

The reformatted `run_test.sh` is the foundation for the entire 
iterative cycle. After this step, the pipeline relies on its output 
in the following ways:

- **Step 2 (golden flows)**: The script is run against the original 
  C program. For each test case, the per-test trace 
  (`flow_results/test{N}_trace.log`) is recorded as the ground truth 
  execution flow.
- **Step 6 (repair)**: The script is run against the translated 
  Rust program. For each test case:
  - If `flow_results/test{N}_success.log` is produced, the test is 
    treated as passing and the corresponding code is left untouched.
  - If `flow_results/test{N}_fail.log` is produced, the test is 
    treated as failing. The pipeline then compares 
    `test{N}_trace.log` against the golden flow to localize the 
    discrepancy and asks the LLM to repair the relevant code.


## Required structure of run_test.sh

### Per-test binary

The original script calls a single binary (e.g., `./build/zopfli`). The reformatted script calls a per-test binary (e.g., `./test_binaries/zopfli_t1`, `./test_binaries/zopfli_t2`, ...). 
This allows golden flows to be recorded separately for each test case, so that the LLM can localize and fix failures on a per-test basis.

### Tracing instrumentation

Each test is executed with `LD_PRELOAD=libtracer.so` and 
`TRACE_OUTPUT` set to a per-test trace log. This captures the 
execution flow of the program for each test case.

```bash
LD_PRELOAD=libtracer.so TRACE_OUTPUT=$PWD/flow_results/test1_trace.log \
    ./test_binaries/zopfli_t1 test_input.txt
```

### Standardized pass/fail reporting

Use a uniform format for reporting results:

```
Test N started
Test N passed    # or: Test N failed
Test N ended
```

Failed tests also write to stderr (`echo "Test N failed" >&2`).

### Individual result logging

Each test writes its output to `flow_results/testN_success.log` or `flow_results/testN_fail.log`, enabling the pipeline to inspect results per test case.


## Option 1: Writing manually

If you write `run_test.sh` from scratch, follow this structure for 
each test case:

```bash
echo "Test N started"
LD_PRELOAD=libtracer.so TRACE_OUTPUT=$PWD/flow_results/testN_trace.log \
    ./test_binaries/{program}_tN [args] > /tmp/testN.out 2>&1
exit_code=$?

if [ $exit_code -eq 0 ] && /* functional check */; then
    cp /tmp/testN.out flow_results/testN_success.log
    echo "Test N passed"
else
    cp /tmp/testN.out flow_results/testN_fail.log
    echo "Test N failed" >&2
    failed=1
fi
echo "Test N ended"
```

Required at the top: `failed=0` and `mkdir -p flow_results`. Required 
at the bottom: `exit $failed`. Each test must use a unique binary 
(`{program}_t1`, `{program}_t2`, ...) built by `c_build.sh`.


## Option 2: LLM-assisted automated reformatting

The LLM-assisted reformatter takes the original test script as input 
and produces `run_test.sh` automatically. 
For benchmark programs, we provide `base_test.sh` under each 
`benchmark/{program}/` directory, which can be used directly as input 
to the reformatter.


After running the reformatter, please review the generated `run_test.sh` to confirm it satisfies the [required structure](#required-structure-of-run_testsh). 
The LLM may produce incorrect results for some programs.


### Command

```bash
cd /root/SmartC2Rust/macro
python3 pre_process.py /root/SmartC2Rust/benchmark/{program} reformat base /root/SmartC2Rust/benchmark/{program}/base_test.sh
```

#### Input

- `benchmark/{program}/base_test.sh` — the original test script

#### Output

- `benchmark/{program}/run_test.sh` — the reformatted test script
- `macro/chats_0000_reformat/{program}/` — LLM interaction prompt logs