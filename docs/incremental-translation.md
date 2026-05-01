# Incremental Translation

Real C codebases are large. Translating everything at once rarely works:
the build breaks, errors pile up, and you can't tell which problems are
worth fixing. Instead, start from a small subset that translates
cleanly, then widen the scope step by step.

---

## The basic loop

1. Translate a subset of the codebase under one configuration (build flags, target, features).
2. Make sure it builds and the tests pass.
3. Widen the subset, or switch to a different configuration. Go to 1.

---


## The mental model

At any point during the migration, the project looks like this:

```
+-----------------+        FFI         +-----------------+
|   Rust crate    | <----------------> |    C sources    |
| (translated so  |   extern "C" fns   | (not yet done)  |
|  far)           |   #[repr(C)] data  |                 |
+-----------------+                    +-----------------+
        \                                       /
         \              linked together        /
          +-------> single executable <-------+
                    or single library
```

Translation proceeds by **moving the boundary inward**: each round, one or
more C functions / files are replaced by Rust equivalents, and the FFI shims
that used to call into C are deleted (or flipped to call from C into Rust,
if there are still C callers).


---

## Scenario A: Switching configuration between runs
### Step 0: Initial run (resume `off`)

```bash
cd /root/SmartC2Rust/trans
python3 compile.py /root/SmartC2Rust/trans/c_code_0000/{program} /root/SmartC2Rust/trans/trans_c_0000/{program} /root/SmartC2Rust/benchmark/{program}/targets_actual.txt trans /root/SmartC2Rust/trans/metadata_0000/{program} /root/SmartC2Rust/trans/div_metadata_0000/{program} database_0000/{program}/block_output.txt off
```


```bash
cd /root/SmartC2Rust/trans
python3 semantics.py s_repair /root/SmartC2Rust/trans/workspace_0000_{program}/{program}
```

### Step 1: Stash the initial run's metadata

Preserve the metadata produced by Step 0 so it can be passed to the next run as the `previous_*` arguments:

```bash
python3 semantics.py \
  stash /root/SmartC2Rust/trans/metadata_0000/{program} \
  /root/SmartC2Rust/trans/div_metadata_0000/{program} \
  /root/SmartC2Rust/trans/database_0000/{program}/block_output.txt
```

The `stash` mode is equivalent to running the following shell commands:

```bash
rm -rf /root/SmartC2Rust/trans/previous_metadata_0000/{program}
cp -r  /root/SmartC2Rust/trans/metadata_0000/{program} \
       /root/SmartC2Rust/trans/previous_metadata_0000/{program}

rm -rf /root/SmartC2Rust/trans/previous_div_metadata_0000/{program}
cp -r  /root/SmartC2Rust/trans/div_metadata_0000/{program} \
       /root/SmartC2Rust/trans/previous_div_metadata_0000/{program}

mv  /root/SmartC2Rust/trans/database_0000/{program}/block_output.txt \
       /root/SmartC2Rust/trans/database_0000/{program}/previous_block_output.txt
```


### Step 2: Reconfigure for the next run

Before re-running, to switch configuration, change build flags / features in your test
  script or source preparation (e.g., `c_build.sh`, `Makefile`).


### Step 3: Generate golden reference

```bash
cd /root/SmartC2Rust/macro
python3 pre_process.py /root/SmartC2Rust/macro/trans_re_0000/{program} golden
```

### Step 4: Apply macro pre-processing

```bash
cd /root/SmartC2Rust/macro
python3 pre_process.py /root/SmartC2Rust/macro/trans_re_0000/{program} macro off /root/SmartC2Rust/macro/trans_re_0000/{program}/run_test.sh /root/SmartC2Rust/benchmark/{program}/targets.txt
```

### Step 5: Generate metadata
```bash
cd /root/SmartC2Rust/trans
python3 pre_process.py /root/SmartC2Rust/macro/trans_c_0000/{program} meta /root/SmartC2Rust/benchmark/{program}/targets.txt /root/SmartC2Rust/macro/metadata_0000/{program} /root/SmartC2Rust/macro/div_metadata_0000/{program} /root/SmartC2Rust/macro/trans_c_0000/{program}
```


### Step 6: Subsequent runs (resume `on`)

Then re-run with the resume flag set to `on`:

```bash
cd /root/SmartC2Rust/trans
python3 compile.py /root/SmartC2Rust/trans/c_code_0000/{program} /root/SmartC2Rust/trans/trans_c_0000/{program} /root/SmartC2Rust/benchmark/{program}/targets_actual.txt trans /root/SmartC2Rust/trans/metadata_0000/{program} /root/SmartC2Rust/trans/div_metadata_0000/{program} database_0000/{program}/block_output.txt on /root/SmartC2Rust/trans/database_0000/{program}/previous_block_output.txt /root/SmartC2Rust/trans/previous_metadata_0000/{program} /root/SmartC2Rust/trans/previous_div_metadata_0000/{program} /root/SmartC2Rust/trans/previous_workspace_s_repair_0000/{program}/workspace_s_repair_0000_{program}
```

**Input:**

When `<resume_flag>` is `on`, the following four additional arguments are required, in order, after the resume flag:

- `<previous_block_output>`: Block output file from the previous run (e.g., `trans/database_0000/avl/previous_block_output.txt`)
- `<previous_metadata_dir>`: Stashed enriched metadata from the previous run (e.g., `trans/previous_metadata_0000/avl`)
- `<previous_div_metadata_dir>`: Stashed block-level metadata from the previous run (e.g., `trans/previous_div_metadata_0000/avl`)
- `<previous_workspace_s_repair_dir>`: Semantics-repair workspace from the previous run, used to carry over already-translated Rust code (e.g., `trans/previous_workspace_s_repair_0000/avl/workspace_s_repair_0000_avl`)



## Scenario B: Widening target functions

### Step 0: Initial run (resume `off`)

```bash
cd /root/SmartC2Rust/trans
python3 compile.py /root/SmartC2Rust/trans/c_code_0000/{program} /root/SmartC2Rust/trans/trans_c_0000/{program} /root/SmartC2Rust/benchmark/{program}/targets_actual.txt trans /root/SmartC2Rust/trans/metadata_0000/{program} /root/SmartC2Rust/trans/div_metadata_0000/{program} database_0000/{program}/block_output.txt off
```


```bash
cd /root/SmartC2Rust/trans
python3 semantics.py s_repair /root/SmartC2Rust/trans/workspace_0000_{program}/{program}
```

### Step 1: Stash the initial run's metadata

Preserve the metadata produced by Step 0 so it can be passed to the next run as the `previous_*` arguments:

```bash
python3 semantics.py \
  stash /root/SmartC2Rust/trans/metadata_0000/{program} \
  /root/SmartC2Rust/trans/div_metadata_0000/{program} \
  /root/SmartC2Rust/trans/database_0000/{program}/block_output.txt
```

The `stash` mode is equivalent to running the following shell commands:

```bash
rm -rf /root/SmartC2Rust/trans/previous_metadata_0000/{program}
cp -r  /root/SmartC2Rust/trans/metadata_0000/{program} \
       /root/SmartC2Rust/trans/previous_metadata_0000/{program}

rm -rf /root/SmartC2Rust/trans/previous_div_metadata_0000/{program}
cp -r  /root/SmartC2Rust/trans/div_metadata_0000/{program} \
       /root/SmartC2Rust/trans/previous_div_metadata_0000/{program}

mv  /root/SmartC2Rust/trans/database_0000/{program}/block_output.txt \
       /root/SmartC2Rust/trans/database_0000/{program}/previous_block_output.txt
```


### Step 2: Reconfigure for the next run

Before re-running, to widen the subset, edit `targets.txt` to add more entry points.


### Step 3: Generate golden reference
```bash
cd /root/SmartC2Rust/macro
python3 pre_process.py /root/SmartC2Rust/macro/trans_re_0000/{program} golden
```


### Step 4: Apply macro pre-processing

```bash
cd /root/SmartC2Rust/macro
python3 pre_process.py /root/SmartC2Rust/macro/trans_re_0000/{program} macro off /root/SmartC2Rust/macro/trans_re_0000/{program}/run_test.sh /root/SmartC2Rust/benchmark/{program}/targets.txt
```

### Step 5: Generate metadata
```bash
cd /root/SmartC2Rust/trans
python3 pre_process.py /root/SmartC2Rust/macro/trans_c_0000/{program} meta /root/SmartC2Rust/benchmark/{program}/targets.txt /root/SmartC2Rust/macro/metadata_0000/{program} /root/SmartC2Rust/macro/div_metadata_0000/{program} /root/SmartC2Rust/macro/trans_c_0000/{program}
```

### Step 6: Subsequent runs (resume `on`)

Then re-run with the resume flag set to `on`:

```bash
cd /root/SmartC2Rust/trans
python3 compile.py /root/SmartC2Rust/trans/c_code_0000/{program} /root/SmartC2Rust/trans/trans_c_0000/{program} /root/SmartC2Rust/benchmark/{program}/targets_actual.txt trans /root/SmartC2Rust/trans/metadata_0000/{program} /root/SmartC2Rust/trans/div_metadata_0000/{program} database_0000/{program}/block_output.txt on /root/SmartC2Rust/trans/database_0000/{program}/previous_block_output.txt /root/SmartC2Rust/trans/previous_metadata_0000/{program} /root/SmartC2Rust/trans/previous_div_metadata_0000/{program} /root/SmartC2Rust/previous_workspace_s_repair_0000/{program}
```