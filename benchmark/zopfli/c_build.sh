#!/bin/bash

rm -rf build
make clean

mkdir build
cd build
#cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=ON ..
cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -DCMAKE_C_COMPILER=clang -DCMAKE_CXX_COMPILER=clang++ -DCMAKE_C_FLAGS="-finstrument-functions -g -gdwarf-4" -DCMAKE_CXX_FLAGS="-finstrument-functions -g -gdwarf-4" ..
bear -- make zopfli libzopfli  # Build only zopfli and libzopfli
