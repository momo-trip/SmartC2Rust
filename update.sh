#!/bin/bash

set -euo pipefail

cd ..
for dir in SmartC2Rust kiso-llm kiso-parser-c kiso-parser-macro kiso-parser-rust kiso-utils; do
    echo "=== $dir ==="
    cd /root/$dir && git pull --no-rebase
done

cd /root/kiso-parser-c/usage_macro_ref_analyzer && ./build.sh
cd /root/kiso-parser-macro/macro_analyzer && ./build.sh
cd /root/kiso-parser-macro/macro_finder && ./build.sh