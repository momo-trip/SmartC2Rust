#!/bin/bash

cd ..
for dir in SmartC2Rust kiso-llm kiso-parser-c kiso-parser-macro kiso-parser-rust kiso-utils; do
    echo "=== $dir ==="
    cd /root/$dir && git pull --no-rebase
done