#!/bin/bash

option=${1:-"build"}

if [[ "$option" == "init" ]]; then
    rm -rf build
    mkdir build
    cd build
    cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=ON \
          -DCMAKE_C_COMPILER=clang \
          -DCMAKE_CXX_COMPILER=clang++ \
          -DCMAKE_C_FLAGS="-finstrument-functions -g -gdwarf-4" \
          -DCMAKE_CXX_FLAGS="-finstrument-functions -g -gdwarf-4" \
          ..
else
    cd build
fi

make zopfli libzopfli