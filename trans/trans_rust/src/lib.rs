
// Mirrors the C `RUSAGE_MEM_TO_KB(x)` macro, whose definition is selected at
// compile time by the GETRUSAGE_RETURNS_{KB,BYTES,PAGES} configuration.
//
// In config.h this build sets `#define GETRUSAGE_RETURNS_KB 1` while leaving
// GETRUSAGE_RETURNS_BYTES and GETRUSAGE_RETURNS_PAGES undefined, so the C macro
// resolves to the no-op `(x)` (RUSAGE values are already in KB). We encode that
// configuration directly here as a local constant so the translation does not
// depend on a bindgen-emitted symbol (bindgen cannot always locate config.h).
const GETRUSAGE_RETURNS_KB: u32 = 1;

fn rust_RUSAGE_MEM_TO_KB(x: u64) -> u64 {
    if GETRUSAGE_RETURNS_KB != 0 {
        // RUSAGE values are already in KB.
        x
    } else {
        // Convert bytes to kilobytes.
        x / 1024
    }
}
