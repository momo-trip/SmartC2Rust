#!/bin/bash

option=${1:-"build"}

if [ "$option" = "init" ]; then
    make distclean
    ./configure
fi

bear -- make