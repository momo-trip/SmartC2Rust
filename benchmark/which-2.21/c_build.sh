#!/bin/bash

option=${1:-"build"}

# export CFLAGS="-fprofile-arcs -ftest-coverage"
# export LDFLAGS="-lgcov --coverage"

if [ "$option" = "init" ]; then
    make distclean 
    CC=clang CFLAGS="-finstrument-functions -g -gdwarf-4" ./configure
fi
bear -- make -j$(nproc)
