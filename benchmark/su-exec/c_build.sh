#!/bin/bash

option=${1:-"build"}

# export CFLAGS="-fprofile-arcs -ftest-coverage"
# export LDFLAGS="-lgcov --coverage"

if [ "$option" = "init" ]; then
    make clean 
fi

bear -- make -j$(nproc)
