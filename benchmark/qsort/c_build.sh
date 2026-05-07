#!/bin/bash

option=${1:-"build"}

if [[ "$option" == "init" ]]; then
      rm -rf bin
      mkdir -p bin
      cd bin

      cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=1 \
            -DCMAKE_C_COMPILER=clang \
            -DCMAKE_CXX_COMPILER=clang++ \
            -DCMAKE_C_FLAGS="-finstrument-functions -g -gdwarf-4" \
            -DCMAKE_CXX_FLAGS="-finstrument-functions -g -gdwarf-4" ..

else
    cd bin
fi

cmake --build .