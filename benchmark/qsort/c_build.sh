#!/bin/bash

rm -rf bin
mkdir -p bin
cd bin

#cmake .. -DCMAKE_EXPORT_COMPILE_COMMANDS=1
cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=1 -DCMAKE_C_COMPILER=clang -DCMAKE_CXX_COMPILER=clang++ ..

cmake --build .