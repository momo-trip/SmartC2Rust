#!/bin/bash

option=${1:-"build"}

# export CFLAGS="-fprofile-arcs -ftest-coverage"
# export LDFLAGS="-lgcov --coverage"

if [ "$option" = "init" ]; then
    make distclean 
    autoreconf -fiv
    CC=clang CFLAGS="-finstrument-functions -g -gdwarf-4" ./configure
fi
bear -- make -j$(nproc)
bear --append -- make tests/time-aux -j$(nproc)

# bear --append -- make check -j$(nproc)
