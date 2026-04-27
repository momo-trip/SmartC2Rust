# SmartC2Rust

Iterative, Feedback-Driven C-to-Rust Translation via Large Language Models for Safety and Equivalence

**Note:** The documentation may not yet cover everything in detail, and the code still contains some traces of trial and error from the development process, so it may not be the easiest to read. I'll continue to clean it up going forward. At the same time, feedback and suggestions are very welcome!

## Setup with Docker

We prepared the pre-built Docker image, which contains the complete environment (Ubuntu 22.04, Python, Rust, Clang, and all dependencies) ready to run.

### Pull the image

```bash
docker pull ghcr.io/momo-trip/smartc2rust
```

### Run

```bash
docker run -it ghcr.io/momo-trip/smartc2rust
```

This drops you into the container with all tools and source code pre-installed at `/root`.

### Updating dependencies

⚠️IMPORTANT⚠️: Please be sure to run the following before anything else. 
SmartC2Rust depends on several kiso-* libraries that are pre-installed in the Docker image. To pull the latest updates for all components:

```bash
cd /root/SmartC2Rust
bash update.sh
```

This runs `git pull` on all repositories:
- [SmartC2Rust](https://github.com/momo-trip/SmartC2Rust): Main translation pipeline
- [kiso-utils](https://github.com/momo-trip/kiso-utils): Utility library
- [kiso-llm](https://github.com/momo-trip/kiso-llm): LLM interaction library
- [kiso-parser-macro](https://github.com/momo-trip/kiso-parser-macro): Macro analyzer
- [kiso-parser-c](https://github.com/momo-trip/kiso-parser-c): C parser and static analyzer
- [kiso-parser-rust](https://github.com/momo-trip/kiso-parser-rust): Rust parser


## Configuration

Create `/root/SmartC2Rust/config.json` with your LLM API credentials:

```json
{
    "llm_choice": "claude",
    "claude_api_key": "<your-api-key>",
    "azure_endpoint": "<your-endpoint-if-applicable>",
    "test_mode": false,
    "average" : 400,
    "ffi_strategy": "minimize"
}
```

| Field | Description |
|-------|-------------|
| `llm_choice` | LLM backend to use: `claude`, `claude_azure` |
| `claude_api_key` | API key for the selected LLM provider |
| `azure_endpoint` | Endpoint URL (required for `claude_azure` backends, otherwise leave empty `""`) |
| `test_mode` | Set `false` for normal use |
| `average` | Maximum number of source lines per translation unit. |
| `ffi_strategy` | Translation strategy: `"preserve"` (C-compatible via FFI) or `"minimize"` (safe, idiomatic Rust; default) |


## Artifact notes
### Macro handling
When scaling to larger programs, performing macro analysis from scratch with LLMs becomes impractical due to cost considerations. Therefore, we introduce a more structured approach by classifying macros into constant and conditional categories based on parser results. The LLM is then used to refine the translated code, ensuring consistency, successful compilation, and integration across translation units.

### FFI strategy
In the paper, we focus on command-line tools, where the entry point can be translated using a minimize strategy. In contrast, when translating library functions in isolation, FFI interfaces are often unavoidable for interoperability with existing C code. Therefore, we provide two modes (`"preserve"` or `"minimize"`) to support both use cases.


## Translation procedure
### Step 1: Prepare inputs
Before running the iterative cycle, prepare two inputs: a 
standardized test script (`run_test.sh`) and an entry point 
specification (`targets.txt`). For benchmark programs, both are 
provided under `benchmark/{program}/`.


#### Step 1.1: Prepare the test script
Prepares a standardized test script (`run_test.sh`) so that the 
subsequent iterative cycle can run automatically. You can either 
write `run_test.sh` manually or generate it using the LLM-assisted 
reformatter.
See [docs/reformat-testcases.md](docs/reformat-testcases.md) for details.

For benchmark programs, an existing test script (`base_test.sh`) is 
provided under each `benchmark/{program}/` directory and can be 
passed to the LLM-assisted reformatter:

```bash
cd /root/SmartC2Rust/macro
python3 pre_process.py /root/SmartC2Rust/benchmark/{program} reformat base /root/SmartC2Rust/benchmark/{program}/base_test.sh
```

**Input (LLM-assisted reformatter):**
- `<c_source_dir>`: Path to the benchmark program directory (e.g., `/root/SmartC2Rust/benchmark/avl`)
- `reformat`: Processing mode — reformats test cases
- `base`: Test type — uses the base test script as input
- `<base_test_script>`: Path to the original test script (e.g., `benchmark/avl/base_test.sh`)

**Output (LLM-assisted reformatter):**
- `<c_source_dir>/run_test.sh`: reformatted test script with individual test cases
- `macro/chats_0000_reformat/{program}/`: LLM interaction prompt logs for the reformatting step

#### Step 1.2: Prepare the entry point specification
Each benchmark program has a `targets.txt` file in `benchmark/{program}/targets.txt` that specifies which C functions to be the entry point. The entry points are the C functions that will be replaced by their translated Rust equivalents and called from C via FFI.
The `targets.txt` lists function names with their source locations in the format:
`function_name:path/to/file.c:start_line:end_line`

**Note:** For the benchmark programs, the entry point is set to the main function.

See [docs/ffi-boundary.md](docs/ffi-boundary.md) for details on how the FFI boundary is designed.


### Step 2: Get golden flows
Executes the original C program to record golden execution flows as the ground truth.
```bash
cd /root/SmartC2Rust/macro
python3 pre_process.py /root/SmartC2Rust/macro/trans_re_0000/{program} golden
```
**Input:**
- `<c_source_dir>`: Path to the reformatted program directory (e.g., `macro/trans_re_0000/avl`)
- `golden`: Processing mode - golden flow extraction

**Output:**
- `<c_source_dir>/golden/`: directory for saving golden execution flows


### Step 3: Pre-processing for parsing
Resolves and analyzes macros, extracting per-file metadata such as function signatures, types, and macro definitions.
```bash
cd /root/SmartC2Rust/macro
python3 pre_process.py /root/SmartC2Rust/macro/trans_re_0000/{program} macro off /root/SmartC2Rust/macro/trans_re_0000/{program}/run_test.sh /root/SmartC2Rust/benchmark/{program}/targets.txt
```

**Input:**
- `<c_source_dir>`: Path to the reformatted program directory (e.g., `macro/trans_re_0000/avl`)
- `macro`: Processing mode — macro analysis and golden flow extraction
- `off`: LLM usage flag — `off` means no LLM calls in this step
- `<run_test_script>`: Path to the reformatted test script (e.g., `macro/trans_re_0000/avl/run_test.sh`)
- `<targets_file>`: Path to the entry point specification (e.g., `benchmark/avl/targets.txt`)


**Output:**
- `macro/trans_c_0000/{program}/`: C source with macros resolved and annotated
- `macro/metadata_0000/{program}/`: per-file metadata (function signatures, types, macros)
- `macro/div_metadata_0000/{program}/`: per-block metadata for translation units


### Step 4: Pre-processing for segmentation
Performs static analysis to build call graphs and dependency information for segmenting the code into translation units.
```bash
cd /root/SmartC2Rust/trans
python3 pre_process.py /root/SmartC2Rust/macro/trans_c_0000/{program} meta /root/SmartC2Rust/benchmark/{program}/targets.txt /root/SmartC2Rust/macro/metadata_0000/{program} /root/SmartC2Rust/macro/div_metadata_0000/{program} /root/SmartC2Rust/macro/trans_c_0000/{program}
```

**Input:**
- `<c_source_dir>`: Path to the macro-processed C source (e.g., `macro/trans_c_0000/avl`)
- `meta`: Processing mode — generates static analysis metadata for translation
- `<targets_file>`: Path to the entry point specification (e.g., `benchmark/avl/targets.txt`)
- `<metadata_dir>`: Per-file metadata from Step 3 (e.g., `macro/metadata_0000/avl`)
- `<div_metadata_dir>`: Per-block metadata from Step 3 (e.g., `macro/div_metadata_0000/avl`)
- `<original_c_dir>`: Path to the original macro-processed source (e.g., `macro/trans_c_0000/avl`)


**Output:**
- `trans/trans_c_0000/{program}/`: C source prepared for translation
- `trans/metadata_0000/{program}/`: enriched metadata (call graphs, dependencies, FFI boundaries)
- `trans/div_metadata_0000/{program}/`: block-level metadata for incremental translation
- `trans/database_0000/{program}/`: translation database
    - `block_output.txt`: Block output file tracking translation units (e.g., `database_0000/avl/block_output.txt`)

### Step 5: Compilation-repair
Translates C code to Rust and iteratively repairs compilation errors using LLM feedback.
```bash
cd /root/SmartC2Rust/trans
python3 compile.py /root/SmartC2Rust/trans/c_code_0000/{program} /root/SmartC2Rust/trans/trans_c_0000/{program} /root/SmartC2Rust/benchmark/{program}/targets_actual.txt trans /root/SmartC2Rust/trans/metadata_0000/{program} /root/SmartC2Rust/trans/div_metadata_0000/{program} database_0000/{program}/block_output.txt off
```

**Input:**
- `<c_code_dir>`: Path to the C source for translation (e.g., `trans/c_code_0000/avl`)
- `<trans_c_dir>`: Path to the pre-processed C source (e.g., `trans/trans_c_0000/avl`)
- `<targets_file>`: Entry points for translation (e.g., `benchmark/avl/targets_actual.txt`)
- `trans`: Processing mode — performs C-to-Rust translation with iterative compilation repair
- `<metadata_dir>`: Enriched metadata from Step 4 (e.g., `trans/metadata_0000/avl`)
- `<div_metadata_dir>`: Block-level metadata from Step 4 (e.g., `trans/div_metadata_0000/avl`)
- `off`: Debug flag
- `<block_output>`: Block file tracking recording translation units  (e.g., `database_0000/avl/block_output.txt`)

**Output:**
- `trans/workspace_0000_{program}/`: workspace containing:
  - `trans_rust/`: translated Rust library crate (src/lib.rs, Cargo.toml)
  - `run_test.sh`: test execution script for the Rust version
  - `run_all.sh`: combined build and test script
- `trans/database_0000/{program}/`: translation database (prompt history, token usage)
- `trans/chats_0000_trans/{program}/`: LLM interaction prompt logs for the compile-repair step


### Step 6: Semantics-repair
Verifies and repairs the semantic equivalence of the translated Rust code by comparing its behavior against the golden flows. Note that this step also fixes compilation errors that arise during the repair process.
```bash
cd /root/SmartC2Rust/trans
python3 semantics.py /root/SmartC2Rust/trans/workspace_0000_{program}/{program} s_repair
```

**Input:**
- `<workspace_dir>`: Path to the translation workspace (e.g., `trans/workspace_0000_avl/avl`)
- `s_repair`: Processing mode — semantic equivalence repair


**Output:**
- `trans/workspace_s_repair_0000_{program}/`: workspace containing:
  - `trans_rust/`: translated Rust library crate (src/lib.rs, Cargo.toml)
  - `run_test.sh`: test execution script for the Rust version
  - `run_all.sh`: combined build and test script
- `trans/chats_0000_c_repair/{program}/`: LLM interaction prompt logs for the semantics-repair step


## Translating your own C project

The Step 1–6 procedure above assumes one of the bundled benchmarks under
`benchmark/`. To translate your own C project, see
[docs/translating-your-project.md](docs/translating-your-project.md), which
covers:

- Project layout requirements
- Writing `targets.txt` and the base test script
- Adapting the Step 1–6 commands to arbitrary paths
- Tips for tuning `average` and choosing an `ffi_strategy`


## LLM model
The default model is Claude Opus 4.7 (Anthropic).

**Note:** Only Claude models are actively maintained and tested. Other LLM backends (GPT, Gemini, Llama) are included in the codebase but have not been recently verified and may not work as expected.



## Repository structure

### SmartC2Rust
```
SmartC2Rust/
├── macro/
│   └── pre_process.py      # Step 1-3: Test reformatting, golden flow extraction, macro pre-processing
├── trans/                 
│   ├── pre_process.py      # Step 4: Static analysis
│   ├── compile.py          # Step 5: Translation and compilation repair
│   ├── semantics.py        # Step 6: Semantic equivalence repair
│   └── template/           # Build templates (build.rs, run_all.sh)
├── benchmark/              # Benchmark C programs with test cases
│   ├── avl/
│   ├── time-1.9/
│   ├── zopfli/
│   └── ...
├── config.json             # LLM API configuration (not tracked by git)
├── setup.sh                # Dependency installation script
├── commands.txt            # Example commands for all benchmarks
├── update.sh               # Pull latest updates for all repositories
└── README.md
```

### External dependencies (pre-installed in Docker)

```
/root/
├── SmartC2Rust/
├── kiso-utils/             # Shared utility functions (file I/O, JSON, path handling)
├── kiso-llm/               # LLM client (Claude, GPT, Bedrock, Databricks)
├── kiso-parser-c/          # C static analyzer (AST, includes, macros, call graph)
│   ├── c_parser_api/       #   Python API
│   ├── include_finder/     #   Header dependency analyzer
│   ├── usage_analyzer/     #   Symbol usage analyzer
│   └── usage_macro_ref_analyzer/  # Macro reference analyzer
├── kiso-parser-rust/       # Rust code parser
│   └── rust_parser_api/    #   Python API
└── kiso-parser-macro/      # Clang-based macro analyzer
    ├── macro_finder/       #   Preprocessor directive tracker
    └── macro_analyzer/     #   Macro definition analyzer
```

## Paper

Paper: [arXiv:2409.10506](https://arxiv.org/abs/2409.10506) (ICSE 2026)
🆕 This work has been accepted at [ICSE 2026](https://conf.researchr.org/details/icse-2026/icse-2026-research-track/217/SmartC2Rust-Iterative-Feedback-Driven-C-to-Rust-Translation-via-Large-Language-Mode).


## Contact
Momoko Shiraishi\
University email: <u>shiraishi@os.is.s.u-tokyo.ac.jp</u>\
(Personal email: <u>momoko.shiraishi36@gmail.com</u>)
