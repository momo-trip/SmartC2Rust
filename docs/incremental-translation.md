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


## How it works
### Initial run (resume `off`)

```
cd /root/SmartC2Rust/trans
python3 compile.py /root/SmartC2Rust/trans/c_code_0000/{program} /root/SmartC2Rust/trans/trans_c_0000/{program} /root/SmartC2Rust/benchmark/{program}/targets_actual.txt trans /root/SmartC2Rust/trans/metadata_0000/{program} /root/SmartC2Rust/trans/div_metadata_0000/{program} database_0000/{program}/block_output.txt off
```

### Subsequent runs (resume `on`)

Before re-running, change one thing:

- To widen the subset, edit `targets.txt` to add more entry points.
- To switch configuration, change build flags / features in your test
  script or source preparation (e.g., `c_build.sh`, `Makefile`).

Then re-run with the resume flag set to `on`:

```
cd /root/SmartC2Rust/trans
python3 compile.py /root/SmartC2Rust/trans/c_code_0000/{program} /root/SmartC2Rust/trans/trans_c_0000/{program} /root/SmartC2Rust/benchmark/{program}/targets_actual.txt trans /root/SmartC2Rust/trans/metadata_0000/{program} /root/SmartC2Rust/trans/div_metadata_0000/{program} database_0000/{program}/block_output.txt on
```