
#!/bin/bash

option=${1:-"build"}

# Create a stub libtracer.so for the native architecture
cat > /tmp/tracer_stub.c << 'STUBEOF'
#include <stdio.h>
#include <stdlib.h>
void __cyg_profile_func_enter(void *this_fn, void *call_site) {
    const char *trace_out = getenv("TRACE_OUTPUT");
    if (trace_out) {
        FILE *f = fopen(trace_out, "a");
        if (f) {
            fprintf(f, "ENTER %p from %p\n", this_fn, call_site);
            fclose(f);
        }
    }
}
void __cyg_profile_func_exit(void *this_fn, void *call_site) {
    const char *trace_out = getenv("TRACE_OUTPUT");
    if (trace_out) {
        FILE *f = fopen(trace_out, "a");
        if (f) {
            fprintf(f, "EXIT %p from %p\n", this_fn, call_site);
            fclose(f);
        }
    }
}
STUBEOF
gcc -shared -fPIC -o "$PWD/libtracer.so" /tmp/tracer_stub.c
rm -f /tmp/tracer_stub.c

# Reconfigure with gcc for native architecture so binaries actually run
make distclean 2>/dev/null
CC=gcc CFLAGS="-finstrument-functions -g -O0" ./configure

if command -v bear >/dev/null 2>&1; then
    bear -- make -j$(nproc)
    bear --append -- make tests/time-aux -j$(nproc)
else
    make -j$(nproc)
    make tests/time-aux -j$(nproc)
fi

# Create unique binary copies for each test case to avoid 'Text file busy' errors
for i in 1 2 3 4; do
    cp -f ./time ./time_t${i}
    cp -f ./tests/time-aux ./tests/time-aux_t${i}
done

