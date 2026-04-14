# FFI Boundary design

## Overview

When translating C code to Rust, the FFI (Foreign Function Interface) boundary is where unsafe code concentrates. 
The design of this boundary affects how much of the translated Rust code can be written as safe Rust. 
Without explicit guidance, LLMs tend to produce C-style Rust code that uses `unsafe` throughout, rather than isolating it at the boundary.

## Entry point as `main`

When the entry point is the `main` function, the FFI boundary can be kept minimal. The key technique is to convert C types into safe Rust types at the boundary, then pass them to a fully safe Rust function:

```rust
use std::ffi::CStr;
use std::os::raw::c_char;

unsafe fn parse_args(argc: i32, argv: *const *const c_char) -> Vec<String> {
    (0..argc as usize)
        .map(|i| {
            let ptr = *argv.add(i);
            if ptr.is_null() {
                String::new()
            } else {
                CStr::from_ptr(ptr).to_string_lossy().into_owned()
            }
        })
        .collect()
}

fn rust_main(args: Vec<String>) -> i32 {
    // Main logic in safe Rust
    0
}

#[unsafe(no_mangle)]
pub extern "C" fn rust_main_wrapper(
    argc: i32,
    argv: *const *const c_char,
) -> i32 {
    let args = unsafe { parse_args(argc, argv) };
    rust_main(args)
}
```

With this pattern, `unsafe` is confined to the wrapper and argument parsing. The actual program logic in `rust_main` is entirely safe Rust.

## Entry point as library functions

For large-scale programs, it may be practical to translate only a subset of functions rather than the entire program. 
In this case, the entry points are individual library functions rather than `main`, and the FFI boundary becomes more pervasive. 
Each translated function needs its own FFI wrapper, and since these functions may receive and return C types (raw pointers, C strings, C structs), `unsafe` code at each boundary is unavoidable.

This is a natural trade-off: incremental translation gives flexibility in choosing what to translate, but at the cost of more FFI boundaries and more `unsafe` code around them.

## LLM behavior

Without explicit instructions about FFI boundary design, LLMs tend to produce Rust code that mirrors the original C code closely, using raw pointers and `unsafe` blocks throughout the translated code. 
Providing the wrapper pattern as part of the translation prompt helps the LLM separate the FFI layer from the core logic, resulting in more idiomatic and safer Rust.
