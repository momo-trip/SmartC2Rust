#!/bin/bash
set -e

cd ..
cd kiso-utils && pip install -e . && cd ..
cd kiso-llm && pip install -e . && cd ..
cd kiso-parser-c && pip install -e . && cd ..
cd kiso-parser-rust && pip install -e . && cd ..


cd kiso-parser-c/c_parser_api
./build_tracer.sh
# ls -la /usr/local/lib/libtracer.so
sudo cp /root/kiso-parser-c/c_parser_api/libtracer.so /usr/local/lib/
sudo ldconfig

cd ../..


cargo install rustfilt
cargo install bindgen-cli
cargo install cargo-modules