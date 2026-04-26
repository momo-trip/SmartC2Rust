## Translating your own C project

This guide explains how to apply SmartC2Rust to a C project of your own,
rather than one of the bundled benchmarks under `benchmark/`.

The translation procedure itself (Steps 1–6) is identical to the one in the
[main README](../README.md). This document only describes what you need to
prepare beforehand and how to adapt the example commands to arbitrary paths.

> **Prerequisites.** You should have already completed the
> [Setup with Docker](../README.md#setup-with-docker)
---

## 1. Overview

To translate your own project, you need to provide three things:

1. A buildable C source tree.
2. An entry-point specification (`targets.txt`).
3. A test script that exercises the program and reports pass/fail.

Once these are in place, you run the same Steps 1–6 from the main README,
substituting the benchmark paths with the paths to your own project.

---

## 2. Project layout

Place your project under `SmartC2Rust/program/`. This is the standard
location for user projects, mirroring how the bundled samples live under
`benchmark/`. Keeping this convention means you can follow the Step 1–6
commands from the main README with only the project name changed.

​```
/root/SmartC2Rust/
├── benchmark/
│   └── ...
└── program/                # Your projects go here
    └── myproj/             # One directory per project
        ├── src/            # Your C source tree (must build as-is)
        │   ├── main.c
        │   └── ...
        ├── targets.txt     # Entry-point specification (you write this)
        ├── run_test.sh     # 
        └── base_test.sh    # Original test script (you write this)
​```

`myproj` is just a placeholder — use whatever name you like for your
project directory. Throughout the rest of this guide, replace `myproj`
with your actual project name.

> **Note:** The internal directory layout under `program/myproj/` (i.e.
> `src/`, `targets.txt`, etc.) follows the same conventions used by the
> bundled benchmarks. Looking at `benchmark/avl/` as a reference is the
> fastest way to understand the expected structure.

---

## 3. Writing `targets.txt`

`targets.txt` lists the C functions that will be translated to Rust and
called from C via FFI. Each line has the form:

```
function_name:path/to/file.c:start_line:end_line
```

- `path/to/file.c` is **relative to your project root** (the directory you
  pass as `<c_source_dir>` in Step 1).
- `start_line` and `end_line` are 1-indexed and inclusive, covering the
  function definition (signature through closing brace).

### Choosing entry points

For command-line tools, the simplest choice is a single line pointing at
`main`. This matches what the bundled benchmarks do and lets the
`minimize` FFI strategy translate the program into idiomatic Rust without
any FFI boundary inside the translated portion.

```
main:src/main.c:42:178
```

For library-style code, list each public function you want translated. In
that case you will likely want `"ffi_strategy": "preserve"` in
`config.json` so the Rust side remains C-callable. See
[`docs/ffi-boundary.md`](./ffi-boundary.md) for how the FFI boundary is
designed.

### Common pitfalls

- Off-by-one line numbers — include the line containing `{` and the line
  containing the matching `}`.
- Paths that are absolute or that include `./` — keep them relative and
  clean.
- Listing functions that are `static inline` in headers — these are not
  good entry points; pick a regular `.c`-defined function instead.

---

## 4. Writing the base test script

The base test script is what Step 1 reformats into individual test cases.
It should:

- Build the original C program (or assume it is already built).
- Run the program on one or more inputs.
- Exit with status `0` on success and non-zero on failure.

A minimal example:

```bash
#!/usr/bin/env bash
set -euo pipefail

cd "$(dirname "$0")"

# Build
make -C src

# Run a few test cases
./src/myproj  input1.txt > out1.txt
diff -q out1.txt expected1.txt

./src/myproj  input2.txt > out2.txt
diff -q out2.txt expected2.txt

echo "OK"
```

### What makes a good base test script

- **Deterministic.** Avoid timestamps, random seeds, or unordered output.
  If unavoidable, normalize the output before diffing.
- **Self-contained.** The script should not depend on environment state
  outside the project directory.
- **Granular.** Multiple small test cases are better than one big one —
  they give the semantics-repair step (Step 6) more signal about which
  behaviors regressed.
- **Fast.** Each iteration of the repair loop re-runs the tests, so long
  test suites significantly increase translation cost and time.

The reformatting step (Step 1) uses the LLM to split this script into
individual cases. See
[`docs/reformat-testcases.md`](./reformat-testcases.md) for details.

---

## 5. Running Steps 1–6 on your project

Below are the same six steps from the main README, with benchmark-specific
paths replaced by `/root/SmartC2Rust/program/myproj`. Adjust the paths to match your setup.

The numeric suffix `_0000` is an iteration index used by the pipeline; it
will increment automatically across runs. The examples below assume the
first run.

### Step 1: Reformat test cases

```bash
cd /root/SmartC2Rust/macro
python3 pre_process.py \
    /root/SmartC2Rust/program/myproj \
    reformat base \
    /root/SmartC2Rust/program/myproj/base_test.sh
```

Produces `/root/SmartC2Rust/program/myproj/run_test.sh`.

### Step 2: Get golden flows

```bash
cd /root/SmartC2Rust/macro
python3 pre_process.py \
    /root/SmartC2Rust/macro/trans_re_0000/myproj \
    golden
```

### Step 3: Pre-processing for parsing

```bash
cd /root/SmartC2Rust/macro
python3 pre_process.py \
    /root/SmartC2Rust/macro/trans_re_0000/myproj \
    macro off \
    /root/SmartC2Rust/macro/trans_re_0000/myproj/run_test.sh \
    /root/SmartC2Rust/program/myproj/targets.txt
```

### Step 4: Pre-processing for segmentation

```bash
cd /root/SmartC2Rust/trans
python3 pre_process.py \
    /root/SmartC2Rust/macro/trans_c_0000/myproj \
    meta \
    /root/SmartC2Rust/program/myproj/targets.txt \
    /root/SmartC2Rust/macro/metadata_0000/myproj \
    /root/SmartC2Rust/macro/div_metadata_0000/myproj \
    /root/SmartC2Rust/macro/trans_c_0000/myproj
```

### Step 5: Compilation-repair

```bash
cd /root/SmartC2Rust/trans
python3 compile.py \
    /root/SmartC2Rust/trans/c_code_0000/myproj \
    /root/SmartC2Rust/trans/trans_c_0000/myproj \
    /root/SmartC2Rust/program/myproj/targets_actual.txt \
    trans \
    /root/SmartC2Rust/trans/metadata_0000/myproj \
    /root/SmartC2Rust/trans/div_metadata_0000/myproj \
    database_0000/myproj/block_output.txt off
```

### Step 6: Semantics-repair

```bash
cd /root/SmartC2Rust/trans
python3 semantics.py \
    /root/SmartC2Rust/trans/workspace_0000_myproj/myproj \
    s_repair
```

The translated Rust crate is at:

```
/root/SmartC2Rust/trans/workspace_s_repair_0000_myproj/myproj/trans_rust/
```

---

## 7. Tuning `config.json` for your project

Two fields in `config.json` are worth thinking about for non-benchmark
projects:

### `average`

The maximum number of source lines per translation unit. Larger values
produce fewer, bigger units (less LLM overhead per unit but more risk of
hitting context limits and harder repair); smaller values produce more,
smaller units (more LLM calls but easier per-unit reasoning).

Rough starting points:

- Small/simple projects (< 1k LOC): `200`–`400`
- Medium projects: `400` (the default used for benchmarks)
- Larger projects with deep call graphs: try `400` first; reduce if the
  compile-repair step struggles to converge.

### `ffi_strategy`

- `"minimize"` (default) — translate into idiomatic safe Rust. Best for
  command-line tools where `main` is the entry point and there is no
  external C consumer.
- `"preserve"` — keep C-compatible signatures via FFI. Use when the
  translated functions must be callable from existing C code, e.g. when
  you are translating a library.

