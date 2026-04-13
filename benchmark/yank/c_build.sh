#!/bin/bash

option=${1:-"build"}

if [ "$option" = "init" ]; then
    make clean
fi

bear -- make CC=clang CFLAGS+="-finstrument-functions -g -gdwarf-4"