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
cp /root/kiso-parser-c/c_parser_api/libtracer.so /usr/local/lib/
# sudo ldconfig

cd ../..


cargo install rustfilt
cargo install bindgen-cli
cargo install cargo-modules


# Macro parser
cd kiso-parser-macro 
./download_clang.sh

cd macro_finder
./build.sh
cd ..

cd macro_analyzer
./build.sh
cd ..

cd ..

# C parser
cd kiso-parser-c
./download_clang.sh

cd include_finder
./build.sh
cd ..

cd usage_analyzer
./build.sh
cd ..

cd usage_macro_ref_analyzer
./build.sh
cd ..

cd ..

