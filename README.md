# SmartC2Rust

Iterative, Feedback-Driven C-to-Rust Translation via Large Language Models for Safety and Equivalence

## Setup with Docker

We prepared the pre-built Docker image, which contains the complete environment (Ubuntu 22.04, Python, Rust, Clang, and all dependencies) ready to run.

### Pull the image

```bash
docker pull ghcr.io/momo-trip/smartc2rust:v1.0
```

### Run

```bash
docker run --rm -it smartc2rust:v1.0
```

This drops you into the container with all tools and source code pre-installed at `/root`.

### Updating dependencies

SmartC2Rust depends on several kiso-* libraries that are pre-installed in the Docker image. To pull the latest updates for all components:

```bash
cd /root
bash update.sh
```

This runs `git pull` on all repositories:
- [SmartC2Rust](https://github.com/momo-trip/SmartC2Rust) — main translation pipeline
- [kiso-utils](https://github.com/momo-trip/kiso-utils) — utility library
- [kiso-llm](https://github.com/momo-trip/kiso-llm) — LLM interaction library
- [kiso-parser-macro](https://github.com/momo-trip/kiso-parser-macro) — Macro analyzer
- [kiso-parser-c](https://github.com/momo-trip/kiso-parser-c) — C parser and static analyzer
- [kiso-parser-rust](https://github.com/momo-trip/kiso-parser-rust) — Rust parser


## Configuration

Create `/root/SmartC2Rust/config.json` with your LLM API credentials:

```json
{
    "llm_choice": "claude",
    "claude_api_key": "<your-api-key>",
    "azure_endpoint": "<your-endpoint-if-applicable>",
    "test_mode": false
}
```

| Field | Description |
|-------|-------------|
| `llm_choice` | LLM backend to use: `claude`, `claude_azure` |
| `claude_api_key` | API key for the selected LLM provider |
| `azure_endpoint` | Endpoint URL (required for `claude_azure` and `gpt_azure` backends, otherwise leave empty `""`) |
| `test_mode` | Set `false` for normal use |


## Entry point selection

Each benchmark program has a `targets.txt` file in `benchmark/{program}/targets.txt` that specifies which C functions to be the entry point. The file lists function names with their source locations in the format:

`function_name:path/to/file.c:start_line`


## Translation procedure
### Step 1: Reformat test cases
```bash
cd /root/SmartC2Rust/macro
python3 pre_process.py /root/SmartC2Rust/benchmark/{program} reformat base /root/SmartC2Rust/benchmark/{program}/base_test.sh
```

### Step 2: Get golden flows
```bash
cd /root/SmartC2Rust/macro
python3 pre_process.py /root/SmartC2Rust/macro/trans_re_0000/{program} macro off /root/SmartC2Rust/macro/trans_re_0000/{program}/run_test.sh /root/SmartC2Rust/macro/benchmark/{program}/targets.txt
```

### Step 3: Macro pre-processing
```bash
cd /root/SmartC2Rust/macro
python3 pre_process.py /root/SmartC2Rust/macro/trans_re_0000/{program} golden
```

### Step 4: Pre-processing
```bash
cd /root/SmartC2Rust/trans
python3 pre_process.py /root/SmartC2Rust/macro/trans_c_0000/{program} meta /root/SmartC2Rust/benchmark/{program}/targets.txt /root/SmartC2Rust/macro/metadata_0000/{program} /root/SmartC2Rust/macro/div_metadata_0000/{program} /root/SmartC2Rust/macro/trans_c_0000/{program}
```

### Step 5: Compilation-repair
```bash
cd /root/SmartC2Rust/trans
python3 compile.py /root/SmartC2Rust/trans/c_code_0000/{program} /root/SmartC2Rust/trans/trans_c_0000/{program} /root/SmartC2Rust/benchmark/{program}/targets_actual.txt trans /root/SmartC2Rust/trans/metadata_0000/{program} /root/SmartC2Rust/trans/div_metadata_0000/{program} database_0000/{program}/block_output.txt off
```

### Step 6: Compilation-repair
```bash
cd /root/SmartC2Rust/trans
python3 semantics.py /root/SmartC2Rust/trans/workspace_0000_{program}/{program} s_repair
```

### LLM model
- claude

## Paper

Paper: [arXiv:2409.10506](https://arxiv.org/abs/2409.10506) (ICSE 2026)
🆕 This work has been accepted at [ICSE 2026](https://conf.researchr.org/details/icse-2026/icse-2026-research-track/217/SmartC2Rust-Iterative-Feedback-Driven-C-to-Rust-Translation-via-Large-Language-Mode).


## License

This project is licensed under the [Creative Commons Attribution 4.0 International License](https://creativecommons.org/licenses/by/4.0/).


## Contact
Momoko Shiraishi\
University email: <u>shiraishi@os.is.s.u-tokyo.ac.jp</u>\
(Personal email: <u>momoko.shiraishi36@gmail.com</u>)








